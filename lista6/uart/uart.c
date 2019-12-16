#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>

#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

// bufor cykliczny
#define BUFF_SIZE 100

typedef volatile struct cbuf {
  bool full;
  bool empty;
  char memo[BUFF_SIZE];
  uint8_t wp; //writepointer
  uint8_t rp;  //readpointer
} cycleBuffer;

void initCycleBuffer(cycleBuffer * cbuff) {
  cbuff->wp = 0;
  cbuff->rp = 0;
  cbuff->full = false;
  cbuff->empty = true;
  for(uint8_t i = 0; i < BUFF_SIZE; i++)
    cbuff->memo[i] = 0;
}

void writeCycleBuffer(cycleBuffer * cbuff, char data) {
  cbuff->memo[cbuff->wp] = data;

  if(++(cbuff->wp) >= BUFF_SIZE) cbuff->wp = 0;
  if(cbuff->wp == cbuff->rp) cbuff->full = true;
  if(cbuff->empty) cbuff->empty = false;
}

char readCycleBuffer(cycleBuffer * cbuff) {
  char res = cbuff->memo[cbuff->rp];

  if(++(cbuff->rp) >= BUFF_SIZE) cbuff->rp = 0;
  if(cbuff->rp == cbuff->wp) cbuff->empty = true;
  
  cbuff->full = false;
  return res;
}
// ---------------

cycleBuffer brx, btx;  //brx - receive, btx - transmit

// inicjalizacja UART
void uart_init() {
  UBRR0 = UBRR_VALUE;
  UCSR0A = 0;
  UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0) | _BV(UDRIE0);
  UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);

  initCycleBuffer(&brx);
  initCycleBuffer(&btx);
}

// transmisja jednego znaku
int uart_transmit(char data, FILE *stream) {
  while(btx.full);
  writeCycleBuffer(&btx, data);
  return 0;
}

// odczyt jednego znaku
int uart_receive(FILE *stream) {
  while(brx.empty);
  return readCycleBuffer(&brx);
}

ISR(USART_RX_vect) {
  char input = UDR0;
  writeCycleBuffer(&brx, input);
  
  if(input == 13) {
    writeCycleBuffer(&btx, '\n');
    writeCycleBuffer(&btx, '\r');
    return;
  }
  writeCycleBuffer(&btx, input);
}

ISR(USART_UDRE_vect) {
  if(!btx.empty)
    UDR0 = readCycleBuffer(&btx);
  else
    UDR0 = 0;
}

FILE uart_file;

int main() {
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  sei();
  // program testowy 1
  // while(1) {
  //   printf("Hello world!\r\n");
  //   _delay_ms(1000);
  // }
  // program testowy 2
  printf("Hello world!\r\n");
  while(1) {
    int16_t a = 1;
    scanf("%"SCNd16, &a);
    printf("Odczytano: %"PRId16"\r\n", a);
  }
}
