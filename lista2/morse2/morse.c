#include <avr/io.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <util/delay.h>

#define LED PB5
#define LED_DDR DDRB
#define LED_PORT PORTB

#define BTN PB4
#define BTN_PIN PINB
#define BTN_PORT PORTB

#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

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

void resetTimer() {
  TCCR1B = 0;          // Stop timer
  TCNT1 = 0;           // Reset counter
}

void startTimer() {
  // Set prescaler to 256 and start the timer
  TCCR1B |= (1 << WGM12) | (1 << CS12);
}

int main() {
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;

  LED_DDR |= _BV(LED);
  LED_PORT &= ~_BV(LED);

  bool dot = true;
  bool pressed = false;

  char buff[20];
  uint8_t pointer = 0;

  while (1) {
    if(BTN_PIN & _BV(BTN)) {
      if(pressed) {
        pressed = false;
        LED_PORT &= ~_BV(LED);
        resetTimer();
        startTimer();
        buff[pointer] = dot ? '.' : '_';
        pointer++;
        dot = true;
      }
      //adding letter
      if(TCNT1 >= 0xF423) {
        resetTimer();
        buff[pointer] = '\0';
        pointer = 0;
        LED_PORT |= _BV(LED);
        _delay_ms(200);
        LED_PORT &= ~_BV(LED);
        // WYSLANIE NA EKRAN ZNAKU

        for(uint8_t i = 0; i < 26; i++) {
          if(strcmp(buff, letters[i]) == 0) {
            printf("%c\r\n", i+97);
            break;
          }
        }
      }
    } else {
      if(!pressed) {
        // Set prescaler to 256 and start the timer
        resetTimer();
        startTimer();
      }

      pressed = true;
      if(TCNT1 >= 0x7A11 && dot) {
        resetTimer();
        dot = false;
        LED_PORT |= _BV(LED);
      }
    }
  }
}