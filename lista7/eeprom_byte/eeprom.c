#include <avr/io.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include "i2c.h"

#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

// inicjalizacja UART
void uart_init() {
  // ustaw baudrate
  UBRR0 = UBRR_VALUE;
  // wyczyść rejestr UCSR0A
  UCSR0A = 0;
  // włącz odbiornik i nadajnik
  UCSR0B = _BV(RXEN0) | _BV(TXEN0);
  // ustaw format 8n1
  UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

// transmisja jednego znaku
int uart_transmit(char data, FILE *stream) {
  // czekaj aż transmiter gotowy
  while(!(UCSR0A & _BV(UDRE0)));
  UDR0 = data;
  return 0;
}

// odczyt jednego znaku
int uart_receive(FILE *stream) {
  // czekaj aż znak dostępny
  while (!(UCSR0A & _BV(RXC0)));
  char data = UDR0;
  printf("%c%s", data, data == 13 ? "\r\n" : "");
  return data;
}

FILE uart_file;

const uint8_t eeprom_addr = 0xa0;

void eeprom_write(uint16_t addr, uint8_t data) {
  #define i2cCheckWrite(code, msg) \
    if ((TWSR & 0xf8) != (code)) { \
      printf(msg " failed, status: %.2x\r\n", TWSR & 0xf8); \
      i2cReset(); \
      return; \
    }
  
  i2cStart();
  i2cCheckWrite(0x8, "I2C start")
  i2cSend(eeprom_addr | ((addr & 0x100) >> 7));
  i2cCheckWrite(0x18, "I2C EEPROM write request")
  i2cSend(addr & 0xff);
  i2cCheckWrite(0x28, "I2C EEPROM set address")
  i2cSend(data);
  i2cStop();
  i2cCheckWrite(0xf8, "I2C stop")
}

uint8_t eeprom_read(uint16_t addr) {
  #define i2cCheckRead(code, msg) \
    if ((TWSR & 0xf8) != (code)) { \
      printf(msg " failed, status: %.2x\r\n", TWSR & 0xf8); \
      i2cReset(); \
      return 0; \
    }
  
  i2cStart();
  i2cCheckRead(0x8, "I2C start")
  i2cSend(eeprom_addr | ((addr & 0x100) >> 7));
  i2cCheckRead(0x18, "I2C EEPROM write request")
  i2cSend(addr & 0xff);
  i2cCheckRead(0x28, "I2C EEPROM set address")
  i2cStart();
  i2cCheckRead(0x10, "I2C second start")
  i2cSend(eeprom_addr | 0x1 | ((addr & 0x100) >> 7));
  i2cCheckRead(0x40, "I2C EEPROM read request")
  uint8_t data = i2cReadNoAck();
  i2cCheckRead(0x58, "I2C EEPROM read")
  i2cStop();
  i2cCheckRead(0xf8, "I2C stop")
  return data;
}

int main() {
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  // zainicjalizuj I2C
  i2cInit();
  
  while(1) {
    char cmd[32];
    uint16_t addr;
    uint8_t val;

    scanf("%s %"SCNu16, cmd, &addr);

    if(strcmp(cmd, "read") == 0) {
      val = eeprom_read(addr);
      printf("R %"PRIu16" => %"PRIu8"\r\n", addr, val);
    } else if(strcmp(cmd, "write") == 0){
      scanf("%"SCNu8, &val);
      eeprom_write(addr, val);
      printf("W %"PRIu8" => %"PRIu16"\r\n", val, addr);
    }
  }
}
