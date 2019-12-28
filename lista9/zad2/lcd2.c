#include <avr/io.h>
#include <util/delay.h>
#include <inttypes.h>
#include "hd44780.h"

int main() {
  // skonfiguruj wyświetlacz
  LCD_Initialize();
  LCD_Clear();

  // NOWE ZNAKI           
  uint8_t curr = 0x20; // 0b00100000
  for(uint8_t addr = 0x00; addr < 0x06; addr++) {
    LCD_WriteCommand(0x40 + (addr * 8));
    for(uint8_t i = 0; i < 8; i++)
      LCD_WriteData(curr & 0x1f); //maska -> 0b00011111
    curr |= curr >> 1;
  }
  // ----------

  // program testowy
  uint8_t cnt1 = 0, cnt2 = 0;
  while(1) {
    LCD_GoTo(cnt2, 0);
    LCD_WriteData(cnt1);
    _delay_ms(100);
    if(++cnt1 > 5) {
      cnt1 = 1;
      if(++cnt2 > 15) {
        cnt2 = 0;
        _delay_ms(1000);
        LCD_Clear();
      }
    }
  }
}