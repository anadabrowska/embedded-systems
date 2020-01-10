#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <inttypes.h>

volatile uint16_t read_value = 0;
uint16_t value = 0;

// inicjalizacja ADC (fotorezystor)
void adc_init() {
  ADMUX   = _BV(REFS0); // referencja AVcc, wejście ADC0
  DIDR0   = _BV(ADC0D); // wyłącz wejście cyfrowe na ADC0

  // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
  ADCSRA  = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
  ADCSRA |= _BV(ADEN) | _BV(ADIE); // włącz ADC, włącz przerwania

  ADCSRB = 0;
  ADCH = 0;
}

// przerwanie adc
ISR(ADC_vect) {
  float v = ((float)ADC) * (50.0 / 1023.0);
  read_value = v + 20;
}

int main() {
  adc_init();

  // OC1A jako wyjście
  DDRB = _BV(PB1);

  // PWM, Phase Correct, preskaler 256
  ICR1 = 625; // ~50Hz
  OCR1A = 20; // [20; 70] 
  TCCR1A = _BV(COM1A1) | _BV(WGM11) | _BV(WGM13);
  TCCR1B = _BV(CS12);

  // włącz przerwania
  sei();
  
  while(1) {
    if(value != read_value) {
      value = read_value;
      OCR1A = value;
    }

    // inicjacja pomiaru
    _delay_ms(100);
    ADCSRA |= _BV(ADSC);
  }
}
