#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <inttypes.h>
#include <stdlib.h>

#define blue PINB1
#define green PINB2
#define red PINB3

uint8_t currVal = 0;
uint8_t currRGB[3]= {0, 0, 0};

void setColor(uint16_t H, uint8_t V) {
  // C = V * 1  ->  S = 1
  // V(0, 1) -> V(0, 255)
	uint8_t X = V * (1 - abs((H/60)%2 - 1));

	if(H >= 0 && H < 60) {
		currRGB[0] = V;
		currRGB[1] = X;
		currRGB[2] = 0;	
	} else if(H >= 60 && H < 120) {	
		currRGB[0] = X;
		currRGB[1] = V;
		currRGB[2] = 0;	
	} else if(H >= 120 && H < 180) {
		currRGB[0] = 0;
		currRGB[1] = V;
		currRGB[2] = X;	
	} else if(H >= 180 && H < 240) {
		currRGB[0] = 0;
		currRGB[1] = X;
		currRGB[2] = V;	
	} else if(H >= 240 && H < 300) {
		currRGB[0] = X;
		currRGB[1] = 0;
		currRGB[2] = V;	
	} else {
		currRGB[0] = V;
		currRGB[1] = 0;
		currRGB[2] = X;	
	}
}

void rgb(uint8_t r, uint8_t g, uint8_t b, uint8_t v) {
	if(v == r){
		PORTB&= ~_BV(red);
	}
	if(v == g){
		PORTB&= ~_BV(green);
	}
	if(v == b){
		PORTB&= ~_BV(blue);
	}
}

// overflow interrupt for timer0
ISR(TIMER0_OVF_vect) {
	if(currVal == 0) {
    PORTB |= _BV(blue) | _BV(green) | _BV(red);
		currVal = 255;
	}
	rgb(currRGB[0], currRGB[1], currRGB[2], currVal);
	currVal--;
}
                     
int main(){
  DDRB = _BV(blue) | _BV(green) | _BV(red);
  PORTB = _BV(blue) | _BV(green) | _BV(red);

  // set up timer registers
	TCCR0A = 0;           // normal mode
	TCCR0B = _BV(CS00);   // no prescaling
	TIMSK0 = _BV(TOIE0);  // overflow interrupt enabled

  // start interrupts
	sei();

  srand(1998); // some seed
  while(1){
    uint16_t h = rand() % 360;
    uint8_t  v = 0;

    while(v < 255/2) { //not to be that light
      v++;
      setColor(h, v);
      _delay_ms(20);
    }

    while(v > 0) {
      v--;
      setColor(h, v);
      _delay_ms(20);
    }

    _delay_ms(300);
  }
}
