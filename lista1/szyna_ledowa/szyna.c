#include <avr/io.h>
#include <util/delay.h>

#define LED_DDR DDRD
#define LED_PORT PORTD

int main() {
  UCSR0B &= ~_BV(RXEN0) & ~_BV(TXEN0);

  LED_DDR |= 0b11111111;

  LED_PORT = 1;
  int dir = 0;
  while (1) {
    switch(dir) {
      case 0:
        LED_PORT <<= 1;
        break;
      case 1:
        LED_PORT >>= 1;
        break;
    }
    _delay_ms(100);

    if((LED_PORT == 0b00000001) | (LED_PORT == 0b10000000)) {
      dir = !dir;
    }
  }
}