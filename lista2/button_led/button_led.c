#include <avr/io.h>
#include <inttypes.h>
#include <stdbool.h>
#include <util/delay.h>

#define LED PB5
#define LED_DDR DDRB
#define LED_PORT PORTB

#define BTN PB4
#define BTN_PIN PINB
#define BTN_PORT PORTB

void execute(uint8_t data) {
  for(uint8_t i=0; i < data; i++) {
    LED_PORT |= _BV(LED);
    _delay_ms(100);
    LED_PORT &= ~_BV(LED);
    _delay_ms(300);
  }
}

int main() {
  LED_DDR |= _BV(LED);
  LED_PORT &= ~_BV(LED);

  uint8_t counter = 0;
  bool released = true;

  while (1) {
    if(BTN_PIN & _BV(BTN)) {
      released = true;
    } else if(released) {
      released = false;
      counter++;
      if(counter == 1) {
        // Set prescaler to 256 and start the timer
        TCCR1B |= (1 << WGM12) | (1 << CS12);
      }
    }

    if(TCNT1 >= 0xF423 && counter > 0) {
      TCCR1B = 0;          // Stop timer
      TCNT1 = 0;           // Reset counter
      execute(counter);
      counter = 0;
    }
  }
}