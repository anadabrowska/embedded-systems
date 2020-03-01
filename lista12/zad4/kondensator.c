#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <inttypes.h>
#include <stdio.h>

#define BAUD 9600                        // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1) // zgodnie ze wzorem

volatile uint32_t counter = 0;

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

// inicjalizacja ADC (kondensator)
void adc_init() {
  ADMUX   = _BV(REFS0) | _BV(MUX1); // referencja AVcc, wejście ADC2
  DIDR0   = _BV(ADC2D); // wyłącz wejście cyfrowe na ADC2

  // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
  ADCSRA  = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
  ADCSRA |= _BV(ADEN); // włącz ADC, włącz przerwania
}

uint16_t adc_get() {
  ADCSRA |= _BV(ADSC);
  while (!(ADCSRA & _BV(ADIF)));
  ADCSRA |= _BV(ADIF);

  return ADC;
}

ISR(TIMER0_OVF_vect) {
  // 16MHz/(1*256) = 62500 razy na sekundę
  counter++;
}

int main() {
  adc_init();

  uart_init();
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;

  DDRB   =  _BV(PB1);
  PORTB &= ~_BV(PB1);

  // timer do mierzenia czasu
  TCCR0A = 0;
  TCCR0B = _BV(CS00);
  TIMSK0 = _BV(TOIE0);

  sei();

  uint32_t value = 0, curr_counter = 0;

  while(1) {
    puts("Wcisnij dowolny przycisk aby rozpoczac pomiar...\r");
    getchar();
    PORTB |= _BV(PB1);
    counter = 0;

    while(value < 500) {
      curr_counter = counter;
      value = adc_get();
    }

    double time = (double)curr_counter / 62500;
    double c = (0.00013 * time) / 2.444;

    uint32_t nF = c * 1000000000.0;

    printf("pojemnosc: %"PRIu32"nF\r\n", nF);

    value = 0;
    PORTB &= ~_BV(PB1);
    _delay_ms(1000);
  }
}