#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdio.h>
#include <inttypes.h>

#include <stdio.h>
#include <util/delay.h>

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

volatile long long time = 0;
volatile uint16_t frequency = 0;
uint16_t edge = 0;

// przerwanie na rising edge
ISR(TIMER1_CAPT_vect) {
  if(edge == 0 || edge == 1){
    edge += 1;
    TCNT1 = 0;
  }else if(edge == 2){
    edge = 3;
    time = TCNT1;
    if(time > 0){
      frequency = 16000000ll / (time * 256ll);
      printf("Czestotliwosc: %"PRIu16" Hz\r\n", frequency);
    }
  }
}

int main() {
  uart_init();
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;

  TCCR1B = _BV(ICES1) | _BV(CS12);  // rising edge trigger, preskaler 256
  TIMSK1 = _BV(ICIE1);              // input capture interrupt włączony

  set_sleep_mode(SLEEP_MODE_IDLE);

  sei();
  
  while(1) {
    sleep_mode();
  }
}