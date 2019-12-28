#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>
#include "hd44780.h"

#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

// inicjalizacja UART
void uart_init() {
  UBRR0 = UBRR_VALUE;
  UCSR0A = 0;
  UCSR0B = _BV(RXEN0) | _BV(TXEN0);
  UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

// transmisja jednego znaku
int lcd_transmit(char data, FILE *stream) {
  LCD_WriteData(data);
  return 0;
}

// odczyt jednego znaku
int uart_receive(FILE *stream) {
  while (!(UCSR0A & _BV(RXC0)));
  return UDR0;
}

FILE stream_file;

int main() {
  // skonfiguruj wyświetlacz
  LCD_Initialize();
  LCD_Clear();
  // skonfiguruj strumienie wejściowe/wyjściowe
  uart_init();
  fdev_setup_stream(&stream_file, lcd_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &stream_file;

  // program testowy
  char buff[32];
  uint8_t index = 0;
  while(1) {
    char c = getchar();
    if(c == 13 || index >= 16) {
      buff[index] = '\0';
      index = 0;
      LCD_Clear();
      LCD_GoTo(0, 0);
      printf("%s", buff);
      LCD_GoTo(0, 1);
      if(c == 13) continue;
    }
    buff[index] = c;
    putchar(c);
    index++;
  }
}
