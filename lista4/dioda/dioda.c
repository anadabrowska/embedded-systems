#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <inttypes.h>

#define TIMER1_PRESCALER (uint8_t) 64
#define LED PINB1

uint8_t currVal = 0;
uint8_t valueLed = 0;

// inicjalizacja ADC (potencjometr)
void adc_init() {
  ADMUX   = _BV(REFS0); // referencja AVcc, wejście ADC0
  DIDR0   = _BV(ADC0D); // wyłącz wejście cyfrowe na ADC0

  // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
  ADCSRA  = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
  ADCSRA |= _BV(ADEN); // włącz ADC
}

uint8_t analogRead() {
  OCR1B = 0; // wyłącz diodę na czas pomiaru

  ADCSRA |= _BV(ADSC); // wykonaj konwersję
  while(ADCSRA & (1<<ADSC)); // czekaj na wynik

  // ADC(0, 1023) -> ADC(0, 254)
  uint16_t v = ADC / 4;
  return 255 - v;
}

// overflow interrupt for timer0
ISR(TIMER0_OVF_vect) {
	if(currVal == 0) {
    PORTB |= _BV(LED);
		currVal = 255;
	}
	if(currVal == valueLed){
		PORTB&= ~_BV(LED);
	}
	currVal--;
}

int main() {
  DDRB = _BV(LED);
  PORTB = _BV(LED);

  // set up timer registers
	TCCR0A = 0;           // normal mode
	TCCR0B = _BV(CS00);   // no prescaling
	TIMSK0 = _BV(TOIE0);  // overflow interrupt enabled

  // start interrupts
	sei();

  // zainicjalizuj ADC
  adc_init();

  while(1) {
    valueLed = analogRead();
    _delay_ms(100);
  }
}