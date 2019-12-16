#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <inttypes.h>
#include <stdio.h> 

#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

#define PIN_SCK  PD4
#define PIN_SS   PD5
#define PIN_MOSI PD6
#define PIN_MISO PD7

// inicjalizacja UART
void uart_init() {
  UBRR0 = UBRR_VALUE;
  UCSR0A = 0;
  UCSR0B = _BV(RXEN0) | _BV(TXEN0);
  UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

// transmisja jednego znaku
int uart_transmit(char data, FILE *stream) {
  while(!(UCSR0A & _BV(UDRE0)));
  UDR0 = data;
  return 0;
}

// odczyt jednego znaku
int uart_receive(FILE *stream) {
  while (!(UCSR0A & _BV(RXC0)));
  return UDR0;
}

FILE uart_file;

// inicjalizacja SPI(slave)
void spi_init() {
  // ustaw pin MISO jako wyjscie
  DDRB |=  _BV(DDB4);
  // SCK i ~SS jako wejścia
  DDRB &= ~_BV(DDB5) & ~_BV(DDB2);
  // włącz SPI
  SPCR = _BV(SPE);
}

// odbieranie bajtu
uint8_t spi_receive() {
  while(!(SPSR & _BV(SPIF)));
  return SPDR;
}

char miso[8];

char* send_number(uint8_t number) {
  uint8_t current, mask = 0b10000000;

  PORTD &= ~_BV(PIN_SS);
  PORTD &= ~_BV(PIN_SCK);
  for(uint8_t i = 0; i < 8; i++) {
    current = (number & mask) >> (7 - i);
    mask >>= 1;

    PORTD = current ? (PORTD | _BV(PIN_MOSI)) : (PORTD & ~_BV(PIN_MOSI));
    miso[i] = (PIND & _BV(PIN_MISO))? '1':'0';

    PORTD |= _BV(PIN_SCK);
    _delay_ms(1);
    PORTD &= ~_BV(PIN_SCK);
  }
  PORTD |= _BV(PIN_SS);
  return miso;
}

int main() {
  // zainicjalizuj UART
  uart_init();
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;

  // zainicjalizuj SPI
  spi_init();

  // symulowanie SPI zwyklymi pinami (recznie)
  DDRD |=  _BV(PIN_MOSI) | _BV(PIN_SCK) | _BV(PIN_SS);
  PORTD &= ~_BV(PIN_SCK);
  PORTD |= _BV(PIN_SS);


  uint8_t number = 0;
  while(1) {
    printf("=> %"PRIu8" ", number);
    printf("<= %s\r\n", send_number(number));
    number++;
    _delay_ms(2000);
  }
}