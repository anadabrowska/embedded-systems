#include <avr/io.h>
#include <util/delay.h>
#include <inttypes.h>

#define TIMER1_PRESCALER (uint8_t) 64

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

  // 1024/8 = 128
  uint16_t v = ADC / 128;
  if(v == 0) {
    return 0;
  } else {
    return _BV(v);
  }
}

int main() {
  // OC1B wyjscie
  DDRB = _BV(PINB2);

  TCCR1A |= _BV(WGM10);
  TCCR1B |= _BV(WGM12);

  TCCR1A |= _BV(COM1B1); // Clear OC1B on Compare Match 
  TCCR1B |= _BV(CS10) | _BV(CS11); // Preksaler = 64,  fpwm = 976,5Hz

  // zainicjalizuj ADC
  adc_init();
  // mierz napięcie
  while(1) {
    OCR1B = analogRead();
    _delay_ms(100);
  }
}