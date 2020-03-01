#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "pid.h"

#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

/*
 * The K_P, K_I and K_D values (P, I and D gains)
 * need to be modified to adapt to the application at hand
 */
#define K_P     150.00
#define K_I     0.05
#define K_D     0.50

volatile int16_t temp_set = 35, referenceValue, measurementValue, inputValue, counter = 0, timer = 0;

/*
 * Flags for status information
 */
volatile struct GLOBAL_FLAGS {
  // True when PID control loop should run one time
  uint8_t pidTimer:1;
  uint8_t dummy:7;
} gFlags = {0, 0};

// Parameters for regulator
struct PID_DATA pidData;

/*
 * Specify the desired PID sample time interval
 * With a 8-bit counter (255 cylces to overflow), the time interval value is calculated as follows:
 * TIME_INTERVAL = ( desired interval [sec] ) * ( frequency [Hz] ) / 255
 */
#define TIME_INTERVAL   157

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

void pwm_init() {
  // PWM, Phase Correct, preskaler 64
  DDRB |= _BV(PB1);
  ICR1 = 256;
  OCR1A = 0;
  TCCR1A = _BV(COM1A1) | _BV(WGM11) ;
  TCCR1B = _BV(CS11) | _BV(CS10) | _BV(WGM13);
}

// inicjalizacja ADC (fotorezystor)
void adc_init() {
  ADMUX = _BV(REFS0) | _BV(REFS1); // ref 1.1V
  DIDR0 = _BV(ADC0D); // wyłącz wejście cyfrowe na ADC0

  // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
  ADCSRA  = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
  ADCSRA |= _BV(ADEN); // włącz ADC
}

void Init(void) {
  pid_Init(K_P * SCALING_FACTOR, K_I * SCALING_FACTOR , K_D * SCALING_FACTOR , &pidData);

  // Set up timer, enable timer/counte 0 overflow interrupt
  TCCR0B = (1<<CS00);
  TIMSK0 = (1<<TOIE0);
  TCNT0 = 0;
}

/*
 * This function must return the reference value.
 * May be constant or varying
 */
int16_t Get_Reference(void) {
  return temp_set * 100;
}

/*
 * This function must return the measured data
 */
int16_t Get_Measurement(void) {
  ADCSRA |= _BV(ADSC);
  while (!(ADCSRA & _BV(ADIF)));
  ADCSRA |= _BV(ADIF);

  float temp_actual = (((((float)ADC)/1024.0) * 1.1) - 0.5) * 100.0;

  return (temp_actual * 100.0);
}

/*
 * Set the output from the controller as input
 * to system.
 */
void Set_Input(int16_t inputValue) {
  uint8_t mapped = (inputValue + 256) / 2;
  OCR1A = mapped;
}

/*
 * Timer interrupt to control the sampling interval
 */
ISR(TIMER0_OVF_vect) {
  static uint16_t i = 0;
  if(i < TIME_INTERVAL) {
    i++;
  } else {
    referenceValue = Get_Reference();
    measurementValue = Get_Measurement();

    inputValue = pid_Controller(referenceValue, measurementValue, &pidData);

    Set_Input(inputValue);
    i = 0;
    counter++;
  }
  if(counter > 300) {
    uint8_t mapped = (inputValue + 256) / 2;
    uint8_t tmp1 = measurementValue / 100;
    uint8_t tmp2 = measurementValue - (tmp1 * 100);
    printf("set: %"PRIu8" | actual: %"PRIu8".%"PRIu8" | mapped: %"PRIu8"\r\n", temp_set, tmp1, tmp2, mapped);
    counter = 0;
  }
}

/*
 * Test program
 */
int main() {
  Init();

  pwm_init();
  adc_init();
  uart_init();
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;

  DDRB |= _BV(PB5);
  PORTB &= ~_BV(PB5);

  OCR1A = 0;
  
  sei();

  while(1) {
    scanf("%"SCNd16, &temp_set);
  }

  return 0;
}
