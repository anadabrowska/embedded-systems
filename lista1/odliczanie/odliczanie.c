#include <avr/io.h>
#include <util/delay.h>

#define LED_DDR DDRD
#define LED_PORT PORTD

char numbers[] = {
  0b01000000, // 0
  0b01111001, // 1
  0b00100100, // 2
  0b00110000, // 3
  0b00011001, // 4
  0b00010010, // 5
  0b00000010, // 6
  0b01111000, // 7
  0b00000000, // 8
  0b00010000, // 9
//   GFEDCBA
};

int main() {
  UCSR0B &= ~_BV(RXEN0) & ~_BV(TXEN0);

  LED_DDR |= 0b11111111;

  int counter = 0;
  while (1) {
    LED_PORT = numbers[counter];
    counter++;
    if(counter >= 10) counter = 0;
    _delay_ms(1000);
  }
}