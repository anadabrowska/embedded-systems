#include "FreeRTOS.h"
#include "task.h"
#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "uart.h"
#include "queue.h"
#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#ifndef BAUD
#define BAUD 9600
#endif
#include <util/setbaud.h>

int uart_transmit(char c, FILE *stream);
int uart_receive(FILE *stream);

QueueHandle_t xQueue_btx, xQueue_brx;

FILE uart_file = FDEV_SETUP_STREAM(uart_transmit, uart_receive, _FDEV_SETUP_RW);

void uart_init() {
  UBRR0H = UBRRH_VALUE;
  UBRR0L = UBRRL_VALUE;
#if USE_2X
  UCSR0A |= _BV(U2X0);
#else
  UCSR0A &= ~(_BV(U2X0));
#endif
  UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
  UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0) | _BV(UDRIE0); /* Enable RX and TX, ISR */

xQueue_btx = xQueueCreate(50, sizeof(char));
xQueue_brx = xQueueCreate(50, sizeof(char));
}

int uart_transmit(char c, FILE *stream) {
  xQueueSendToBack(xQueue_btx, &c, 0);
  return 0;
  // while (!(UCSR0A & _BV(UDRE0))) taskYIELD();
  // UDR0 = c;
  // return 0;
}

int uart_receive(FILE *stream) {
  char received;
  if(xQueueReceive(xQueue_brx, &received, portMAX_DELAY) == pdPASS);
    return  received;
  // while (!(UCSR0A & _BV(RXC0))) taskYIELD();
  // return UDR0;
}

ISR(USART_RX_vect) {
  char input = UDR0;
  char new = '\n';
  char back = '\r';
  BaseType_t xTaskWokenByReceive = pdFALSE;
  xQueueSendToBackFromISR(xQueue_brx, &input, &xTaskWokenByReceive);

  if(input == 13) {
    xQueueSendToBackFromISR(xQueue_btx, &new, &xTaskWokenByReceive);
    xQueueSendToBackFromISR(xQueue_btx, &back, &xTaskWokenByReceive);
    return;
  }
  xQueueSendToBackFromISR(xQueue_btx, &input, 0);
  if( xTaskWokenByReceive != pdFALSE )
    {
      taskYIELD ();
    }
}

ISR(USART_UDRE_vect) {
  char received;
  BaseType_t xTaskWokenByReceive = pdFALSE;
  if(xQueueReceiveFromISR(xQueue_brx, &received, &xTaskWokenByReceive) == pdPASS)
    UDR0 = received;
  if( xTaskWokenByReceive != pdFALSE )
    {
      taskYIELD ();
    }
}

