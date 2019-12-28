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

volatile uint8_t temp_set = 28;
volatile float temp_actual = 0;

// inicjalizacja ADC (fotorezystor)
void adc_init() {
  ADMUX = _BV(REFS0) | _BV(REFS1); // ref 1.1V
  DIDR0 = _BV(ADC0D); // wyłącz wejście cyfrowe na ADC0

  // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
  ADCSRA  = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
  ADCSRA |= _BV(ADEN) | _BV(ADIE); // włącz ADC, włącz przerwania

  ADCSRB = 0;
  ADCH = 0;
}

void print_data() {
  uint8_t tmp1 = temp_actual;
  uint8_t tmp2 = (temp_actual - tmp1) * 100.0;
  printf("set: %"PRIu8" | actual: %"PRIu8".%"PRIu8"\r\n", temp_set, tmp1, tmp2);
}

// przerwanie adc
ISR(ADC_vect) {
  temp_actual = (((((float)ADC)/1024.0) * 1.1) - 0.5) * 100.0;

  if((float)temp_actual < temp_set - 1.0) {
    PORTB |= _BV(PB5);
  } else if((float)temp_actual > temp_set) {
    PORTB &= ~_BV(PB5);
  }
}

uint8_t interrupts = 0;
ISR(TIMER0_OVF_vect) {
  if(interrupts >= 100) {
    interrupts = 0;
    print_data();
    return;
  } else if(interrupts == 50 || interrupts == 0) {
    ADCSRA |= _BV(ADSC);
  }

  interrupts++;
}

int main() {
  uart_init();
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;

  DDRB |= _BV(PB5);
  PORTB |= _BV(PB5);

  TIMSK0 |= _BV(TOIE0);            // przerwanie overflow
  TCCR0B |= _BV(CS02) | _BV(CS00); // preskaler 1024
  // 16Mhz * 2^4 * 1024 = +- 10ms

  adc_init();
  sei();
  
  while(1) {
    scanf("%"SCNu8, &temp_set);
  }
}
