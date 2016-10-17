#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define VOICES 4

volatile unsigned int osc[VOICES];
volatile unsigned int step[VOICES];

volatile unsigned char next = 0;

ISR(TIMER1_OVF_vect) {
  PORTD = next;
  long sample = 0;
  for( int i=0; i<VOICES; i++ ) {
    unsigned int v = osc[i] + step[i];
    osc[i] = v;
    sample += v;
  }
  next = sample >> 10;
}

void play( int voice, unsigned int s ) {
  step[voice] = s;
  if( s == 0 ) {
    osc[voice] = 0;
  }
}

int main(void) {

  TCCR1A = 0; // Reset control registers timer1 /not needed, safety
  TCCR1B = 0; // Reset control registers timer1 // not needed, safety
  TIMSK1 = 0b00000001; //timer1 output compare match and overflow interrupt enable
  OCR1A = 320; // Set TOP/compared value (your value) (maximum is 65535 i think)
  TCCR1B = 0b00001001;  // prescaling=8 CTC-mode (two counts per microsecond)

  TCCR1A = 0b01000000;  //set OCR1A on compare match (output high level level)
  DDRB |= 0b00000010;  // SET OC1A=PB1= digital pin 9 to output//  OC1A = PB1 (Arduino digital pin #9)
  // SEEMS THAT GETTING BOTH A INTERRUPT (TIMER1_COMPA_vect) AND a output pinchange isnt possible.. I havent managed // to anyway

  DDRD = 0xFF;
  play( 0, 25 );
  play( 1, 60 );
  play( 2, 100 );
  play( 3, 220 );

  for( ;; ) { }

}
