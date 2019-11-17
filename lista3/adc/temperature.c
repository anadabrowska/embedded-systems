#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>
#include <math.h>

#define K2C 273
#define T0  297
#define R0  4700

#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

// inicjalizacja UART
void uart_init()
{
  // ustaw baudrate
  UBRR0 = UBRR_VALUE;
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

// inicjalizacja ADC
void adc_init()
{
  ADMUX   = _BV(REFS0) | _BV(MUX0); // referencja AVcc, wejście ADC1
  DIDR0   = _BV(ADC1D); // wyłącz wejście cyfrowe na ADC1

  // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
  ADCSRA  = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
  ADCSRA |= _BV(ADEN); // włącz ADC
}

FILE uart_file;

double temp() {
  double R = 4400.0;
  double B = 3540.0;

  //double x = log((double)R / (double)R0) / B + (1/(double)T0);
  double t = (double)ADC*R ;
  t = t/(1024.0 - (double)ADC); //R
  t = log((double)R0/t);
  t = t * (double)T0;
  t = t + B;
  t = ((double)T0*B) / t;
  return t - (double)K2C;
}

int main(){
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  // zainicjalizuj ADC
  adc_init();
  // mierz napięcie
  while(1) {
    ADCSRA |= _BV(ADSC); // wykonaj konwersję
    while(ADCSRA & (1<<ADSC)); // czekaj na wynik

    double t = temp();
    int16_t tc = t;
    uint8_t tu = (t*10) - (tc*10);
    printf("Temperatura [C]: %"PRId16".%"PRIu8" \r\n", tc, tu);
    _delay_ms(1000);
  }
}