#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_pwr.h"

#include "uart1_printf.hpp"

extern "C"
{

uint8_t ucHeap[configTOTAL_HEAP_SIZE] __attribute__ (( section(".ram_d2_s1_noload") ));

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

void vApplicationIdleHook(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);

void vApplicationIdleHook( void )
{
   /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
   to 1 in FreeRTOSConfig.h. It will be called on each iteration of the idle
   task. It is essential that code added to this hook function never attempts
   to block in any way (for example, call xQueueReceive() with a block time
   specified, or call vTaskDelay()). If the application makes use of the
   vTaskDelete() API function (as this demo application does) then it is also
   important that vApplicationIdleHook() is permitted to return to its calling
   function, because it is the responsibility of the idle task to clean up
   memory allocated by the kernel to any task that has since been deleted. */

  // TODO: turn this back on
  // HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
}

void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
	for(;;)
	{
		uart1_log<64>(LOG_LEVEL::FATAL, "freertos", "Stack Overflow!");
		vTaskDelay(500);
	}
}

void vApplicationMallocFailedHook(void)
{
	for(;;)
	{
		uart1_log<64>(LOG_LEVEL::FATAL, "freertos", "Malloc Failed!");
		vTaskDelay(500);
	}
}

static StaticTask_t xIdleTaskTCBBuffer __attribute__(( section(".ram_dtcm_noload") ));
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE] __attribute__(( section(".ram_dtcm_noload") ));
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}

}