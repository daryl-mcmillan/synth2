#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "serial.h"

#define VOICES 4

struct oscillator {
  volatile unsigned int phase;
  volatile unsigned int rate;
  volatile unsigned char volume;
  volatile unsigned char mask;
};

volatile oscillator oscx[VOICES];

volatile unsigned char next = 0;

volatile unsigned int time = 0;

ISR(TIMER1_COMPA_vect) {
  PORTD = next;
  char sample = 0;
  for( int i=0; i<VOICES; i++ ) {
    unsigned int v = oscx[i].phase + oscx[i].rate;
    unsigned char v_high = v >> 8;
    v_high &= oscx[i].mask;
    oscx[i].phase = v;
    unsigned char result;
    asm(
      "mul %1, %2\n"
      "mov %0, r1\n"
      : "=r" (result)
      : "r" (oscx[i].volume), "r" (v_high)
      : "r0" , "r1"
    );
    sample += result;
  }
  next = sample;
  time++;
}

const unsigned int notes[] = {
  11, 11, 12, 13, 14, 14, 15, 16, 17, 18, 19, 20,
  21, 23, 24, 25, 27, 29, 30, 32, 34, 36, 38, 40,
  43, 45, 48, 51, 54, 57, 61, 64, 68, 72, 76, 81,
  86, 91, 96, 102, 108, 114, 121, 128, 136, 144, 153, 162,
  171, 182, 192, 204, 216, 229, 242, 257, 272, 288, 306, 324,
  343, 363, 385, 408, 432, 458, 485, 514, 544, 577, 611, 647,
  686, 727, 770, 816, 864, 915, 970, 1028, 1089, 1153, 1222, 1295,
  1372, 1453, 1540, 1631, 1728, 1831, 1940, 2055, 2177, 2307, 2444, 2589,
  2743, 2906, 3079, 3262, 3456, 3662, 3880, 4110, 4355, 4614, 4888, 5179,
  5487, 5813, 6159, 6525, 6913, 7324, 7759, 8221, 8710, 9227, 9776, 10357,
  10973, 11626, 12317, 13050, 13826, 14648, 15519, 16441
};

void play(
    unsigned char voice,
    unsigned int rate,
    unsigned char volume,
    unsigned char mask
  ) {
  oscx[voice].rate = rate;
  oscx[voice].volume = volume;
  oscx[voice].mask = mask;
  if( rate == 0 ) {
    oscx[voice].phase = 0;
  }
}

int main(void) {

  TCCR1A = 0; // clear control register
  TCCR1B = 0; // clear control register
  TIMSK1 = 0b00000010; //timer1 OC1A interrupt enabled
  OCR1A = 320; // Set TOP
  TCCR1B = 0b00001001;  // prescaling=1 CTC mode
  TCCR1A = 0b01000000;  // toggle pin on OCR1A
  DDRB |= 0b00000010;  // enabled output
  sei();

  DDRD = 0xFF;

  serialSetup( 31250 );

  char voice_mask = 0;

  const unsigned char voice_lookup[] = {
    3, 3, 3, 3, 3, 3, 3, 3,
    2, 2, 2, 2,
    1, 1,
    0
  };

  unsigned char note_voices[128];
  for( int i=0; i<128; i++ ) {
    note_voices[i] = 0;
  }
  for( ;; ) {
    unsigned char cmd = serialRead();
    if( cmd == 0x80 ) {
      unsigned char note = serialRead();
      if( note & 0x80 ) continue;
      unsigned char vol = serialRead();
      if( vol & 0x80 ) continue;
      unsigned char voice = note_voices[ note ];
      if( voice ) {
        voice--;
        play( voice, 0, 0, 0 );
        note_voices[ note ] = 0;
        voice_mask &=  ~ (1 << voice);
      }
    } else if( cmd == 0x90 ) {
      unsigned char note = serialRead();
      if( note & 0x80 ) continue;
      unsigned char vol = serialRead();
      if( vol & 0x80 ) continue;
      vol = vol >> 1;
      if( note_voices[note] ) {
        continue;
      }
      if( voice_mask < 16 ) {
        unsigned char voice = voice_lookup[voice_mask];
        note_voices[ note ] = voice + 1;
        voice_mask |= 1 << voice;
        play( voice, notes[note], vol, 0xFF );
      }
    }
  }
}
