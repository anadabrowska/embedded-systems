#include <avr/io.h>
#include <util/delay.h>

#define LED PB5
#define LED_DDR DDRB
#define LED_PORT PORTB

void timerStart() {
  // Set the target value to 62499
  OCR1A = 0xF423;
  // Set prescaler to 256 and start the timer
  TCCR1B |= (1 << WGM12) | (1 << CS12);
}

int main() {
  LED_DDR |= _BV(LED);

  timerStart();

  while (1) {
    

    // Waiting for the overflow event
    if ((TIFR1 & (1 <<  OCF1A)) != 0) {
      TIFR1 |= (1<<OCF1A);         //clear the flag
      LED_PORT ^= _BV(LED);
      timerStart();
    }
  }
}