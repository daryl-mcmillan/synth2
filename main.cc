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
}

void play(
    int voice,
    unsigned int freq10,
    unsigned char volume,
    unsigned char mask
  ) {
  oscx[voice].rate = long(freq10) * 65536L / 500000L;
  oscx[voice].volume = volume;
  oscx[voice].mask = mask;
  if( freq10 == 0 ) {
    oscx[voice].phase = 0;
  }
}

int main(void) {

  TCCR1A = 0; // Reset control registers timer1 /not needed, safety
  TCCR1B = 0; // Reset control registers timer1 // not needed, safety
  TIMSK1 = 0b00000010; //timer1 output compare match and overflow interrupt enable
  OCR1A = 320; // Set TOP/compared value (your value) (maximum is 65535 i think)
  TCCR1B = 0b00001001;  // prescaling=8 CTC-mode (two counts per microsecond)

  TCCR1A = 0b01000000;  //set OCR1A on compare match (output high level level)
  DDRB |= 0b00000010;  // SET OC1A=PB1= digital pin 9 to output//  OC1A = PB1 (Arduino digital pin #9)
  // SEEMS THAT GETTING BOTH A INTERRUPT (TIMER1_COMPA_vect) AND a output pinchange isnt possible.. I havent managed // to anyway
  sei();

  DDRD = 0xFF;
  //play( 0, 61 );
  //play( 1, 60 );
  play( 0, 2220, 64, 0xFF );
  play( 1, 2200, 64, 0xFF );
  play( 2, 440, 64, 0xFF );
  play( 3, 445, 64, 0xFF );
  //play( 3, 0 );

  for( ;; ) {
  }

  //oscx[2].mask = 0xFF;

  //long i = 0;
  //for( ;; ) {
  //  i++;
  //  oscx[2].volume = ((i >> 9) & 0b1111111);
  //  oscx[2].mask = i >> 14;
  //}

}
