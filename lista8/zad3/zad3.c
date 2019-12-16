/******************************************************************************
 * Header file inclusions.
 ******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#include "uart.h"

/******************************************************************************
 * Private macro definitions.
 ******************************************************************************/

#define mainTASK_INOUT_PRIORITY 2
#define mainTASK_LED_PRIORITY 1

/******************************************************************************
 * Private function prototypes.
 ******************************************************************************/
static void vTaskInOut(void* pvParameters);
static void vTaskLed(void* pvParameters);

int main(void) {

  xTaskHandle inOut_handle;
  xTaskHandle led_handle;

  xTaskCreate
    (
      vTaskInOut,
      "TASKInOut",
      configMINIMAL_STACK_SIZE,
      NULL,
      mainTASK_INOUT_PRIORITY,
      &inOut_handle
    );

  xTaskCreate
    (
      vTaskLed,
      "TASKLed",
      configMINIMAL_STACK_SIZE,
      NULL,
      mainTASK_LED_PRIORITY,
      &led_handle
    );

  // Start scheduler.
  vTaskStartScheduler();

  return 0;
}

/******************************************************************************
 *
 ******************************************************************************/
void vApplicationIdleHook(void) {

}

/******************************************************************************
 * Private function definitions.
 ******************************************************************************/

static void vTaskInOut(void* pvParameters) {
  uart_init();
  sei();
  stdin = stdout = stderr = &uart_file;

  char c;
  for(;;) {
    c = getchar();
    putchar(c);
  }
}

/******************************************************************************
 * task 2
 ******************************************************************************/
#define LED PB0
#define LED_DDR DDRB
#define LED_PORT PORTB

static void vTaskLed(void* pvParameters) {
  LED_DDR |= _BV(LED);
  LED_PORT &= ~_BV(LED);

  for(;;) {
    LED_PORT |= _BV(LED);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    LED_PORT &= ~_BV(LED);
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}
