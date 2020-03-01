#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>

#define BAUD 9600                        // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1) // zgodnie ze wzorem

volatile int32_t volume = 0;
volatile uint32_t sum = 0, counter = 0;

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

// inicjalizacja ADC (mikrofon)
void adc_init() {
  ADMUX   = _BV(REFS0) | _BV(MUX0); // referencja AVcc, wejście ADC1
  DIDR0   = _BV(ADC1D); // wyłącz wejście cyfrowe na ADC1

  // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
  ADCSRA  = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
  ADCSRA |= _BV(ADEN); // włącz ADC, włącz przerwania
}

ISR(TIMER0_OVF_vect) {
  // inicjowanie pomiaru 16MHz/(8*256) = 7812.5 ~ 8kHz
  ADCSRA |= _BV(ADSC);
  while (!(ADCSRA & _BV(ADIF)));
  ADCSRA |= _BV(ADIF);

  counter++;

  int32_t val = ADC;
  val -= 512;

  sum += val * val;
  if(counter >= 20) {
    volume = 20.0*log10(sqrt((double)sum / 20.0)/512.0);

    sum = 0;
    counter = 0;
  }
}

ISR(TIMER1_OVF_vect) {
  printf("volume: %"PRId32"dB\r\n", volume);
}

int main() {
  adc_init();

  uart_init();
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;

  // timer do pomiaru
  TCCR0A = 0;
  TCCR0B = _BV(CS01);
  TIMSK0 = _BV(TOIE0);

  // timer do wypisywania
  TCCR1A = 0;
  TCCR1B = _BV(CS12);
  TIMSK1 = _BV(TOIE1);

  set_sleep_mode(SLEEP_MODE_IDLE);
  sei();

  while(1) {
    sleep_mode();
  }
}