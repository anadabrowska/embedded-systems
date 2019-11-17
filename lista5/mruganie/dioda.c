#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stdbool.h>
#include <inttypes.h>

#define LED PD3
#define LED_DDR DDRD
#define LED_PORT PORTD

#define BTN PD2
#define BTN_PIN PIND
#define BTN_PORT PORTD

volatile uint16_t blinks = 0;
volatile uint16_t interrupts = 0;
volatile bool released = true;
volatile bool started = false;

void blink(uint16_t n) {
  for(uint16_t i = 0; i < n; i++) {
    LED_PORT |= _BV(LED);
    _delay_ms(200);
    LED_PORT &= ~_BV(LED);
    _delay_ms(200);
  }
}

ISR(TIMER0_OVF_vect) {
  if(interrupts >= 100) {
    interrupts = 0;
    started = false;
    blink(blinks);
    blinks = 0;
    return;
  }

  if(BTN_PIN & _BV(BTN)) {
    released = true;
  } else if(released) {
    released = false;
    started = true;
    blinks++;
  }

  if(started) interrupts++;
}

int main() {
  set_sleep_mode(SLEEP_MODE_IDLE);

  LED_DDR |= _BV(LED);

  TIMSK0 |= _BV(TOIE0);            // przerwanie overflow
  TCCR0B |= _BV(CS02) | _BV(CS00); // preskaler 1024
  // 16Mhz * 2^4 * 1024 = +- 10ms

  sei();

  while(1) {
    sleep_mode();
  }
}
