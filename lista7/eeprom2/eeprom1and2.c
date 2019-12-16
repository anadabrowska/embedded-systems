#include <avr/io.h>
#include <util/delay.h>
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

const uint8_t eeprom_addr = 0xa0;

#define i2cCheck(code, msg) \
  if ((TWSR & 0xf8) != (code)) { \
    printf(msg " failed, status: %.2x\r\n", TWSR & 0xf8); \
    i2cReset(); \
    return; \
  }

uint16_t last_addr = 0;
char started = 0;

void eeprom_write(uint16_t addr, uint8_t data) {
  if(last_addr + 1 == addr) {
    if(started == 1) {
      i2cSend(data);
      last_addr = addr;
      return;
    }
  } else if(started == 1) {
    started = 0;
    i2cStop();
    i2cCheck(0xf8, "I2C stop")
    _delay_ms(5);
  }

  i2cStart();
  i2cCheck(0x8, "I2C start")
  i2cSend(eeprom_addr | ((addr & 0x100) >> 7));
  i2cCheck(0x18, "I2C EEPROM write request")
  i2cSend(addr & 0xff);
  i2cCheck(0x28, "I2C EEPROM set address")
  i2cSend(data);

  started = 1;
  last_addr = addr;
}

void eeprom_read(uint16_t addr, uint8_t * data) {
  if(started == 1) {
    i2cStop();
    i2cCheck(0xf8, "I2C stop")
    _delay_ms(5);
  }
  i2cStart();
  i2cCheck(0x8, "I2C start")
  i2cSend(eeprom_addr | ((addr & 0x100) >> 7));
  i2cCheck(0x18, "I2C EEPROM write request")
  i2cSend(addr & 0xff);
  i2cCheck(0x28, "I2C EEPROM set address")
  i2cStart();
  i2cCheck(0x10, "I2C second start")
  i2cSend(eeprom_addr | 0x1 | ((addr & 0x100) >> 7));
  i2cCheck(0x40, "I2C EEPROM read request")
  * data = i2cReadNoAck();
  i2cCheck(0x58, "I2C EEPROM read")
  i2cStop();
  i2cCheck(0xf8, "I2C stop")
}

void eeprom_start_read(uint16_t addr) {
  if(started == 1) {
    i2cStop();
    i2cCheck(0xf8, "I2C stop")
    _delay_ms(5);
  }
  i2cStart();
  i2cCheck(0x8, "I2C start")
  i2cSend(eeprom_addr | ((addr & 0x100) >> 7));
  i2cCheck(0x18, "I2C EEPROM write request")
  i2cSend(addr & 0xff);
  i2cCheck(0x28, "I2C EEPROM set address")
  i2cStart();
  i2cCheck(0x10, "I2C second start")
  i2cSend(eeprom_addr | 0x1 | ((addr & 0x100) >> 7));
  i2cCheck(0x40, "I2C EEPROM read request")
}

void eeprom_read_next(uint8_t * data, char last) {
  if(last == 1) {
    * data = i2cReadNoAck();
    i2cCheck(0x58, "I2C EEPROM read")
  } else {
    * data = i2cReadAck();
    i2cCheck(0x50, "I2C EEPROM read")
  }
}

void eeprom_stop_read() {
  i2cStop();
  i2cCheck(0xf8, "I2C stop")
}

int main() {
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  // zainicjalizuj I2C
  i2cInit();
  
  char writing = 0;

  while(1) {
    char cmd[100], c;
    uint16_t idx = 0;
    do {
      c = getchar();
      cmd[idx] = c;
      idx++;
    }while(c != 13);
    cmd[idx] = '\0';

    if(strncmp(cmd, "read", 4) == 0) {
      uint16_t addr, len;
      int argc = sscanf(cmd, "%s %"SCNu16" %"SCNu16, cmd, &addr, &len);
      if(argc == 2) {
        uint8_t val;
        eeprom_read(addr, &val);
        printf("R %"PRIu16" => %"PRIu8"\r\n", addr, val);
      } else if(argc == 3) {
        printf("R [%"PRIu16", %"PRIu16"] => \r\n", addr, addr + len);
        eeprom_start_read(addr);
        for(uint16_t i = 0; i < len; i++) {
          uint8_t data;
          eeprom_read_next(&data, i == len - 1 ? 1 : 0);
          printf("\t:09%03x00%03x%02x\r\n", addr+i, data, 9+addr+i+data);
        }
        eeprom_stop_read();
      }
    } else if(strncmp(cmd, "write", 5) == 0) {
      uint16_t addr;
      uint8_t val;
      int argc = sscanf(cmd, "%s %"SCNu16" %"SCNu8, cmd, &addr, &val);
      if(argc == 3) {
        eeprom_write(addr, val);
        printf("W %"PRIu8" => %"PRIu16"\r\n", val, addr);
      } else if(argc == 1) {
        writing = 1;
        continue;
      }
    } else if(strncmp(cmd, ":", 1) == 0 && writing == 1) {
      uint16_t a, b, c, d;
      sscanf(cmd, ":%2x%3x00%3x%2x", &a, &b, &c, &d);
      eeprom_write(b, c);
      continue;
    }

    if(writing == 1) {
      writing = 0;
    }
  }
}
