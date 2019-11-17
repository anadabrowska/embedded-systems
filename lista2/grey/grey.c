#include <avr/io.h>
#include <stdbool.h>

#define LED_DDR DDRD
#define LED_PORT PORTD

#define BTN_PIN PINB
#define BTN_PORT PORTB

#define BTN0 PB4
#define BTN1 PB3
#define BTN2 PB2

int main() {
  UCSR0B &= ~_BV(RXEN0) & ~_BV(TXEN0);

  LED_DDR |= 0b11111111;
  LED_PORT = 0;

  bool state[] = {false, false, false};
  uint8_t number = 0;

  while (1) {
    // RESET
    if(!(BTN_PIN & _BV(BTN0))) {
      if(!state[0]){
        state[0] = true;
        LED_PORT = 0;
        number = 0;
      }
    } else if(state[0]){
      state[0] = false;
    }

    // PREV
    if(!(BTN_PIN & _BV(BTN1))) {
      if(!state[1]) {
        state[1] = true;
        //number = number > 0 ? number - 1 : 255;
        number--;
        LED_PORT = number ^ (number >> 1);
      }
    } else {
      state[1] = false;
    }

    // NEXT
    if(!(BTN_PIN & _BV(BTN2))) {
      if(!state[2]) {
        state[2] = true;
        //number = number < 255 ? number + 1 : 0;
        number++;
        LED_PORT = number ^ (number >> 1);
      }
    } else if(state[2]){
      state[2] = false;
    }
  }
}