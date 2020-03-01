#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>
#include <math.h>

#define K2C 273
#define T0  297
#define R0  4700

#define BAUD 9600                        // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1) // zgodnie ze wzorem

void uart_init() {
  UBRR0 = UBRR_VALUE;
  UCSR0A = 0;
  UCSR0B = _BV(RXEN0) | _BV(TXEN0);
  UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

int uart_transmit(char data, FILE *stream) {
  while(!(UCSR0A & _BV(UDRE0)));
  UDR0 = data;
  return 0;
}

int uart_receive(FILE *stream) {
  while (!(UCSR0A & _BV(RXC0)));
  return UDR0;
}

FILE uart_file;

// inicjalizacja ADC
void adc_init() {
  ADMUX   = _BV(REFS1) | _BV(REFS0); // referencja 1.1V, wejście ADC0
  DIDR0   = _BV(ADC0D); // wyłącz wejście cyfrowe na ADC0

  // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
  ADCSRA  = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
  ADCSRA |= _BV(ADEN); // włącz ADC
}

double temp() {
  double R = 4400.0;
  double B = 10540.0;

  ADCSRA |= _BV(ADSC); // wykonaj konwersję
  while(ADCSRA & (1<<ADSC)); // czekaj na wynik
  ADCSRA |= _BV(ADIF);

  // double x = log((double)R / (double)R0) / B + (1/(double)T0);
  uint16_t v = ADC;
  v = 1023 - v;
  double t = (double)v*R ;
  t = t/(1024.0 - (double)v); // R
  t = log((double)R0/t);
  t = t * (double)T0;
  t = t + B;
  t = ((double)T0*B) / t;
  return t - (double)K2C;
}

int main() {
  adc_init();

  uart_init();
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  
  uint8_t counter = 0;
  int32_t sum = 0;

  while(1) {
    double v = temp();
    sum += v*v;
    counter++;
    if(counter >= 20) {
      double t = sqrt(sum/20);
      int16_t tc = t;
      uint8_t tu = (t*10) - (tc*10);
      printf("Temperatura [C]: %"PRId16".%"PRIu8" \r\n", tc, tu);
      
      counter = 0;
      sum = 0;
    }
    _delay_ms(50);
  }
}