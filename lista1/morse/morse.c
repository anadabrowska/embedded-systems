#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>

#define LED PB5
#define LED_DDR DDRB
#define LED_PORT PORTB

#define dot   300
#define line  800

#define char_space 1000

#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

// inicjalizacja UART
void uart_init()
{
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
int uart_transmit(char data, FILE *stream)
{
  // czekaj aż transmiter gotowy
  while(!(UCSR0A & _BV(UDRE0)));
  UDR0 = data;
  return 0;
}

// odczyt jednego znaku
int uart_receive(FILE *stream)
{
  // czekaj aż znak dostępny
  while (!(UCSR0A & _BV(RXC0)));
  return UDR0;
}

FILE uart_file;

const char *letters[] = {
  "._",   // A
  "_...", // B
  "_._.", // C
  "_..",  // D
  ".",    // E
  ".._.", // F
  "__.",  // G
  "....", // H
  "..",   // I
  ".___", // J
  "_._",  // K
  "._..", // L
  "__",   // M
  "_.",   // N
  "___",  // O
  ".__.", // P
  "__._", // Q
  "._.",  // R
  "...",  // S
  "_",    // T
  ".._",  // U
  "..._", // V
  ".__",  // W
  "_.._", // X
  "_.__", // Y
  "__..", // Z
};

void signal(char param) {
  LED_PORT |= _BV(LED);
  if(param == '.')
    _delay_ms(dot);
  else if(param == '_')
    _delay_ms(line);
  LED_PORT &= ~_BV(LED);
  _delay_ms(300);
}

void letter(char c) {
  int8_t index = c - 97;

  int8_t i = 0;
  do{
    signal(letters[index][i]);
    i++;
  }while(letters[index][i] != '\0');

  _delay_ms(char_space);
}

void word(const char * w) {
  int8_t i = 0;
  do{
    letter(w[i]);
    i++;
  }while(w[i] != '\0');
}

int main() {
  LED_DDR |= _BV(LED);

  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;

  while(1) {
    char buff[256];
    scanf("%s", buff);
    printf("Odczytano: %s\r\n", buff);
    printf("Morsowanie...");
    word(buff);
    printf("[DONE]\r\n");
  }
}