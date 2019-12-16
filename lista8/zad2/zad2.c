/******************************************************************************
 * Header file inclusions.
 ******************************************************************************/

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include <avr/io.h>
#include <stdio.h>

#include "uart.h"

/******************************************************************************
 * Private macro definitions.
 ******************************************************************************/

#define mainLED_TASK_PRIORITY    2
#define mainSERIAL_TASK_PRIORITY 1

/******************************************************************************
 * Private function prototypes.
 ******************************************************************************/
static void vLed(void* pvParameters);
static void vSerial(void* pvParameters);

QueueHandle_t xQueue;

int main(void) {
  // Queue
  xQueue = xQueueCreate(25, sizeof(uint16_t));

  if(xQueue != NULL) {
    // Create task.
    xTaskCreate
      (
        vLed,
        "led",
        configMINIMAL_STACK_SIZE,
        NULL,
        mainLED_TASK_PRIORITY,
        NULL
      );

    xTaskCreate
      (
        vSerial,
        "serial",
        configMINIMAL_STACK_SIZE,
        NULL,
        mainSERIAL_TASK_PRIORITY,
        NULL
      );

    // Start scheduler.
    vTaskStartScheduler();
  }

  for(;;);

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

#define LED PB0
#define LED_DDR DDRB
#define LED_PORT PORTB

static void vLed(void* pvParameters) {
  LED_DDR |= _BV(LED);
  LED_PORT &= ~_BV(LED);

  uint16_t received;

  for(;;) {
    if(xQueueReceive(xQueue, &received, 10) == pdPASS) {
      LED_PORT |= _BV(LED);
      vTaskDelay(received / portTICK_PERIOD_MS);
      LED_PORT &= ~_BV(LED);
      vTaskDelay(500 / portTICK_PERIOD_MS);
    }
  }
}

/******************************************************************************
 * task 2
 ******************************************************************************/

static void vSerial(void* pvParameters) {
  uart_init();
  stdin = stdout = stderr = &uart_file;
  uint16_t input;
  char c;

  for(;;) {
    input = 0;
    for(;;) {
      c = getchar();
      putchar(c);
      if(c == 13) {
        putchar('\r');
        putchar('\n');
        break;
      };
      input = input * 10 + (c - 48);
    }

    xQueueSendToBack(xQueue, &input, 0);
  } 
}
