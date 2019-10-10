#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>

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

int main() {
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;

  while(1) {
    int8_t a8 = 1, b8 = 1;
    int16_t a16 = 1, b16 = 1;
    int32_t a32 = 1, b32 = 1;
    int64_t a64 = 1, b64 = 1;
    float af = 1, bf = 1;
    printf("[int8_t]\r\n");
    scanf("%"SCNd8" %"SCNd8, &a8, &b8);
    printf("Wynik dodwawania int8_t: %"PRId8"\r\n", a8+b8);
    printf("Wynik mnozenia int8_t: %"PRId8"\r\n", a8*b8);
    printf("Wynik dzielenia int8_t: %"PRId8"\r\n", a8/b8);
    printf("[int16_t]\r\n");
    scanf("%"SCNd16" %"SCNd16, &a16, &b16);
    printf("Wynik dodwawania int16_t: %"PRId16"\r\n", a16+b16);
    printf("Wynik mnozenia int16_t: %"PRId16"\r\n", a16*b16);
    printf("Wynik dzielenia int16_t: %"PRId16"\r\n", a16/b16);
    printf("[int32_t]\r\n");
    scanf("%"SCNd32" %"SCNd32, &a32, &b32);
    printf("Wynik dodwawania int32_t: %"PRId32"\r\n", a32+b32);
    printf("Wynik mnozenia int32_t: %"PRId32"\r\n", a32*b32);
    printf("Wynik dzielenia int32_t: %"PRId32"\r\n", a32/b32);
    printf("[float]\r\n");
    // scanf("%f %f", &af, &bf);
    // printf("Wynik dodwawania float: %f\r\n", af+bf);
    // printf("Wynik mnozenia float: %f\r\n", af*bf);
    // printf("Wynik dzielenia float: %f\r\n", af/bf);

  }
}