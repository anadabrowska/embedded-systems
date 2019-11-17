#include <avr/io.h>
#include <util/delay.h>

#define LED_DDR DDRD
#define LED_PORT PORTD

#define TR_DDR DDRC
#define TR_PORT PORTC

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

void resetTimer() {
  TCCR1B = 0;          // Stop timer
  TCNT1 = 0;           // Reset counter
}

void startTimer() {
  // Set prescaler to 256 and start the timer
  TCCR1B |= (1 << WGM12) | (1 << CS12);
}

int main() {
  UCSR0B &= ~_BV(RXEN0) & ~_BV(TXEN0);

  TR_DDR |= _BV(PC0) | _BV(PC1);

  LED_DDR |= 0b11111111;

  uint8_t counter1 = 0;
  uint8_t counter2 = 0;
  uint8_t display = 0;

  startTimer();
  while (1) {
    if(display == 0) {
      TR_PORT = 0b00000010;
      LED_PORT = numbers[counter1];
    } else if(counter2 != 0) {
      TR_PORT = 0b00000001;
      LED_PORT = numbers[counter2];
    }
    
    if(TCNT1 % 100 == 0) {
      display = !display;
    }
    
    if(TCNT1 >= 0xF423) {
      resetTimer();
      counter1++;
      if(counter1 >= 10) {
        counter1 = 0;
        counter2++;
      }
      if(counter2 >= 6) counter2 = 0;
      startTimer();
    }
  }
}