#include <avr/io.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include "i2c.h"

#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

// inicjalizacja UART
void uart_init() {
  UBRR0 = UBRR_VALUE;
  UCSR0A = 0;
  UCSR0B = _BV(RXEN0) | _BV(TXEN0);
  UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

// transmisja jednego znaku
int uart_transmit(char data, FILE *stream) {
  while(!(UCSR0A & _BV(UDRE0)));
  UDR0 = data;
  return 0;
}

// odczyt jednego znaku
int uart_receive(FILE *stream) {
  while (!(UCSR0A & _BV(RXC0)));
  char data = UDR0;
  printf("%c%s", data, data == 13 ? "\r\n" : "");
  return data;
}

FILE uart_file;

const uint8_t rtc_addr = 0x68;

#define i2cCheck(code, msg) \
  if ((TWSR & 0xf8) != (code)) { \
    printf(msg " failed, status: %.2x\r\n", TWSR & 0xf8); \
    i2cReset(); \
    return; \
  }

void get_date(uint8_t * d, uint8_t * m, uint16_t * y) {
  *d = 0; *m = 0; *y = 0; // w razie failu wyzerowane
  i2cStart();
  i2cCheck(0x8, "I2C start")
  i2cSend(rtc_addr << 1);
  i2cCheck(0x18, "I2C EEPROM write request")
  i2cSend(0x04); // start address (d -> m -> y)
  i2cCheck(0x28, "I2C EEPROM set address")
  i2cStart();
  i2cCheck(0x10, "I2C second start")
  i2cSend((rtc_addr << 1) | 1);
  i2cCheck(0x40, "I2C EEPROM read request")
  uint8_t data_d = i2cReadAck();
  i2cCheck(0x50, "I2C EEPROM read")
  uint8_t data_m = i2cReadAck();
  i2cCheck(0x50, "I2C EEPROM read")
  uint8_t data_y = i2cReadNoAck();
  i2cCheck(0x58, "I2C EEPROM read")
  i2cStop();
  i2cCheck(0xf8, "I2C stop")

  data_m &= 0x7f;
  *d = (data_d & 0xf) + ((data_d >> 4) * 10);
  *m = (data_m & 0xf) + ((data_m >> 4) * 10);
  *y = (data_y & 0xf) + ((data_y >> 4) * 10) + 2000;
}

void get_time(uint8_t * h, uint8_t * m, uint8_t * s) {
  *h = 0; *m = 0; *s = 0; // w razie failu wyzerowane
  i2cStart();
  i2cCheck(0x8, "I2C start")
  i2cSend(rtc_addr << 1);
  i2cCheck(0x18, "I2C EEPROM write request")
  i2cSend(0x00); // start address (s -> m -> h)
  i2cCheck(0x28, "I2C EEPROM set address")
  i2cStart();
  i2cCheck(0x10, "I2C second start")
  i2cSend((rtc_addr << 1) | 1);
  i2cCheck(0x40, "I2C EEPROM read request")
  uint8_t data_s = i2cReadAck();
  i2cCheck(0x50, "I2C EEPROM read")
  uint8_t data_m = i2cReadAck();
  i2cCheck(0x50, "I2C EEPROM read")
  uint8_t data_h = i2cReadNoAck();
  i2cCheck(0x58, "I2C EEPROM read")
  i2cStop();
  i2cCheck(0xf8, "I2C stop")

  *s = (data_s & 0xf) + ((data_s >> 4) * 10);
  *m = (data_m & 0xf) + ((data_m >> 4) * 10);
  *h = (data_h & 0xf) + ((data_h >> 4) * 10);
}

void set_date(uint8_t d, uint8_t m, uint16_t y) {
  y-=2000;
  i2cStart();
  i2cCheck(0x8, "I2C start")
  i2cSend(rtc_addr << 1);
  i2cCheck(0x18, "I2C EEPROM write request")
  i2cSend(0x04); // start address (s -> m -> h)
  i2cCheck(0x28, "I2C EEPROM set address")

  i2cSend(((d / 10) << 4) | ((d - (d / 10) * 10) & 0xf));
  i2cSend(((m / 10) << 4) | ((m - (m / 10) * 10) & 0xf));
  i2cSend(((y / 10) << 4) | ((y - (y / 10) * 10) & 0xf));

  i2cStop();
  i2cCheck(0xf8, "I2C stop")
  printf("setting date to %"PRIu8"-%"PRIu8"-%"PRIu16"\r\n", d, m, 2000+y);
}

void set_time(uint8_t h, uint8_t m, uint8_t s) {
  i2cStart();
  i2cCheck(0x8, "I2C start")
  i2cSend(rtc_addr << 1);
  i2cCheck(0x18, "I2C EEPROM write request")
  i2cSend(0x00); // start address (s -> m -> h)
  i2cCheck(0x28, "I2C EEPROM set address")

  i2cSend(((s / 10) << 4) | ((s - (s / 10) * 10) & 0xf));
  i2cSend(((m / 10) << 4) | ((m - (m / 10) * 10) & 0xf));
  i2cSend(((h / 10) << 4) | ((h - (h / 10) * 10) & 0xf));

  i2cStop();
  i2cCheck(0xf8, "I2C stop")
  printf("time set to %"PRIu8":%"PRIu8":%"PRIu8"\r\n", h, m, s);
}

int main() {
  // zainicjalizuj UART
  uart_init();
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;

  // zainicjalizuj I2C
  i2cInit();
  
  while(1) {
    char cmd[32], opt[32];
    scanf("%s", cmd);

    if(strcmp(cmd, "date") == 0) {
      uint8_t d, m;
      uint16_t y;
      get_date(&d, &m, &y);
      printf("%"PRIu8"-%"PRIu8"-%"PRIu16"\r\n", d, m, y);
    } else if(strcmp(cmd, "time") == 0) {
      uint8_t h, m, s;
      get_time(&h, &m, &s);
      printf("%"PRIu8":%"PRIu8":%"PRIu8"\r\n", h, m, s);
    } else if(strcmp(cmd, "set") == 0) {
      scanf("%s", opt);
      if(strcmp(opt, "date") == 0) {
        uint8_t d, m;
        uint16_t y;
        scanf("%"SCNu8"-%"SCNu8"-%"SCNu16, &d, &m, &y);
        set_date(d, m, y);
      } else if(strcmp(opt, "time") == 0) {
        uint8_t h, m, s;
        scanf("%"SCNu8":%"SCNu8":%"SCNu8, &h, &m, &s);
        set_time(h, m, s);
      }
    }
  }
}
