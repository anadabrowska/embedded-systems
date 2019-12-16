/******************************************************************************
 * Header file inclusions.
 ******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"

#include <avr/io.h>
#include <stdbool.h>

#include <stdio.h>
#include "uart.h"

/******************************************************************************
 * Private macro definitions.
 ******************************************************************************/

#define mainBUTTON_TASK_PRIORITY 1
#define mainLED_TASK_PRIORITY    1

/******************************************************************************
 * Private function prototypes.
 ******************************************************************************/
static void vButton(void* pvParameters);
static void vLed(void* pvParameters);

int main(void) {
  // Create task.
  xTaskHandle button_handle;
  xTaskHandle led_handle;

  xTaskCreate
    (
      vButton,
      "button",
      configMINIMAL_STACK_SIZE,
      NULL,
      mainBUTTON_TASK_PRIORITY,
      &button_handle
    );

  xTaskCreate
    (
      vLed,
      "led",
      configMINIMAL_STACK_SIZE,
      NULL,
      mainLED_TASK_PRIORITY,
      &led_handle
    );

  // Start scheduler.
  vTaskStartScheduler();

  return 0;
}

/**************************************************************************//**
 * \fn static vApplicationIdleHook(void)
 *
 * \brief
 ******************************************************************************/
void vApplicationIdleHook(void)
{

}

/******************************************************************************
 * Private function definitions.
 ******************************************************************************/

#define LED PB0
#define LED_DDR DDRB
#define LED_PORT PORTB

#define BTN PB3
#define BTN_PIN PINB
#define BTN_PORT PORTB

static void vButton(void* pvParameters) {
  LED_DDR |= _BV(LED);
  LED_PORT &= ~_BV(LED);

  uint16_t delayCounter = 0;
  uint8_t counter = 0;
  bool released = true, started = false;

  for(;;) {
    if(BTN_PIN & _BV(BTN)) {
      released = true;
    } else if(released) {
      released = false;
      counter++;
      if(counter == 1) {
        started = true;
      }
    }

    if(started) {
      vTaskDelay(10 / portTICK_PERIOD_MS);
      delayCounter++;
      if(delayCounter >= 100) {
        for(uint8_t i = 0; i < counter; i++) {
          LED_PORT |= _BV(LED);
          vTaskDelay(100 / portTICK_PERIOD_MS);
          LED_PORT &= ~_BV(LED);
          vTaskDelay(300 / portTICK_PERIOD_MS);
        }

        started = false;
        delayCounter = 0;
        counter = 0;
      }
    }

  }
}

/**************************************************************************//**
 * task 2
 ******************************************************************************/
#define EYE_DDR DDRD
#define EYE_PORT PORTD

static void vLed(void* pvParameters) {
  UCSR0B &= ~_BV(RXEN0) & ~_BV(TXEN0);

  EYE_DDR |= 0b11111111;

  EYE_PORT = 1;
  int dir = 0;
  for(;;) {
    switch(dir) {
      case 0:
        EYE_PORT <<= 1;
        break;
      case 1:
        EYE_PORT >>= 1;
        break;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);

    if((EYE_PORT == 0b00000001) | (EYE_PORT == 0b10000000)) {
      dir = !dir;
    }
  }
}
