#define TIMER1_PRESCALER (uint8_t) 1

#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>

#define FREQUENCY 37900 // 37.9kHz

#define LED PB5
#define LED_DDR DDRB
#define LED_PORT PORTB

#define SENSOR PB0
#define SENSOR_PIN PINB

int main(void) {
  LED_DDR |= _BV(LED);
  LED_PORT |= _BV(LED);

  // Set OC1A as output pin
  DDRB |= _BV(PINB1);

  // Set Timer1, mode CTC, toggle on compare, prescale 1
  TCCR1A = _BV(COM1A0);
  TCCR1B = _BV(WGM12) | _BV(CS11) | _BV(CS10);

  while(1) {
    OCR1A = (F_CPU / (FREQUENCY * TIMER1_PRESCALER * 2) - 1);
    _delay_us(650);
    OCR1A = 0;
    _delay_us(650);

    if(!(SENSOR_PIN & _BV(SENSOR))) {
      LED_PORT |= _BV(LED);
    } else {
       LED_PORT &= ~_BV(LED);
    }
  }
}