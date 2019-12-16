#include <avr/io.h>
#include <util/delay.h>

#define LA PB1
#define OE PB2

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

// inicjalizacja SPI
void spi_init() {
  // ustaw piny MOSI, SCK i ~SS jako wyjścia
  DDRB |= _BV(DDB3) | _BV(DDB5) | _BV(DDB2);
  // włącz SPI w trybie master z zegarem 250 kHz
  SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPR1);
}

// transfer jednego bajtu
uint8_t spi_transfer(uint8_t data) {
  // rozpocznij transmisję
  SPDR = data;
  // czekaj na ukończenie transmisji
  while (!(SPSR & _BV(SPIF)));
  // wyczyść flagę przerwania
  SPSR |= _BV(SPIF);
  // zwróć otrzymane dane
  return SPDR;
}

int main() {
  spi_init();
  DDRB |= _BV(OE) | _BV(LA);
  PORTB &= ~_BV(OE);

  uint8_t counter = 0;
  while(1) {
    spi_transfer(~numbers[counter]);
    PORTB |= _BV(LA);
    _delay_us(10);
    PORTB &= ~_BV(LA);
    if(++counter > 9) counter = 0;
    _delay_ms(1000);
  }
}
