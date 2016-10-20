#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

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
    unsigned int freq10,
    unsigned char volume,
    unsigned char mask
  ) {
  oscx[voice].rate = long(freq10) * 2048L / 15625L;
  oscx[voice].volume = volume;
  oscx[voice].mask = mask;
  if( freq10 == 0 ) {
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

  for( ;; ) {
    play( 0, time/8, 64, 0xFF );
    play( 1, (time + 10000) / 8, 64, 0xFF );
    play( 2, (time + 20000) / 8, 64, 0xFF );
    play( 3, (65535 - time) / 8, 64, 0xFF );
  }

  //oscx[2].mask = 0xFF;

  //long i = 0;
  //for( ;; ) {
  //  i++;
  //  oscx[2].volume = ((i >> 9) & 0b1111111);
  //  oscx[2].mask = i >> 14;
  //}

}
