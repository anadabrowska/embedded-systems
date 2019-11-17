#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>

#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

void uart_init() {
  UBRR0 = UBRR_VALUE;
  UCSR0A = 0;
  UCSR0B = _BV(RXEN0) | _BV(TXEN0);
  UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

int uart_transmit(char data, FILE *stream) {
  while(!(UCSR0A & _BV(UDRE0)));
  UDR0 = data;
  return 0;
}

int uart_receive(FILE *stream) {
  while (!(UCSR0A & _BV(RXC0)));
  return UDR0;
}

FILE uart_file;

volatile uint16_t resistance = 0;

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

// przerwanie dla przycisku
ISR (INT0_vect) {
  // inicjacja pomiaru
  ADCSRA |= _BV(ADSC);
}

// przerwanie adc
ISR(ADC_vect) {
  // todo: pomiar ohm'ow
  resistance = (10000 * ADC) / (1024 - ADC);
}

int main() {
  uart_init();
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;

  DDRD &= ~_BV(DDD2);    // PD2 wejście przerwania
  PORTD &= ~_BV(PORTD2); // bez wbudowanego pull up'a

  EICRA |= _BV(ISC01);   // przerwanie dla opadającej krawędzi
  EIMSK |= _BV(INT0);    // włącz INT0

  adc_init();

  sei();                 // włącz przerwania
  
  while(1) {
    printf("Rezystancja: %"PRIu16" Ohm\r\n", resistance);
    _delay_ms(1000);
  }
}
