#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

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

void play(
    int voice,
    unsigned long freq10,
    unsigned char volume,
    unsigned char mask
  ) {
  oscx[voice].rate = freq10 * 2048L / 15625L;
  oscx[voice].volume = volume;
  oscx[voice].mask = mask;
  if( freq10 == 0 ) {
    oscx[voice].phase = 0;
  }
}

unsigned long notes[] = {
  82, 87, 92, 97, 103, 109, 116, 122, 130, 138, 146, 154,
  164, 173, 184, 194, 206, 218, 231, 245, 260, 275, 291, 309,
  327, 346, 367, 389, 412, 437, 462, 490, 519, 550, 583, 617,
  654, 693, 734, 778, 824, 873, 925, 980, 1038, 1100, 1165, 1235,
  1308, 1386, 1468, 1556, 1648, 1746, 1850, 1960, 2077, 2200, 2331, 2469,
  2616, 2772, 2937, 3111, 3296, 3492, 3700, 3920, 4153, 4400, 4662, 4939,
  5233, 5544, 5873, 6223, 6593, 6985, 7400, 7840, 8306, 8800, 9323, 9878,
  10465, 11087, 11747, 12445, 13185, 13969, 14800, 15680, 16612, 17600, 18647, 19755,
  20930, 22175, 23493, 24890, 26370, 27938, 29600, 31360, 33224, 35200, 37293, 39511,
  41860, 44349, 46986, 49780, 52740, 55877, 59199, 62719, 66449, 70400, 74586, 79021,
  83720, 88698, 93973, 99561, 105481, 111753, 118398, 125439
};

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
  for( ;; ) {
    unsigned char cmd = serialRead();
    if( cmd == 0x80 ) {
      play( 0, 0, 0, 0 );
    } else if( cmd == 0x90 ) {
      unsigned char note = serialRead();
      unsigned char vol = serialRead();
      play( 0, notes[note], vol, 0xFF );
    }
  }

  unsigned char note = 0;
  unsigned int last_time = time;
  for( ;; ) {
    unsigned int t = time;
    if( (t ^ last_time) & 0b1110000000000000 ) {
      note ++;
      play( 0, notes[note & 0b1111111], 64, 0x80 );
    }
    last_time = t;
    //play( 1, (time + 10000) / 8, 64, 0xFF );
    //play( 2, (time + 20000) / 8, 64, 0xFF );
    //play( 3, (65535 - time) / 8, 64, 0xFF );
  }

  //oscx[2].mask = 0xFF;

  //long i = 0;
  //for( ;; ) {
  //  i++;
  //  oscx[2].volume = ((i >> 9) & 0b1111111);
  //  oscx[2].mask = i >> 14;
  //}

}
