#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

#include "common_util/Stack_string.hpp"

#include "hal_inst.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_pwr.h"

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
  HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
}

void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
	for(;;)
	{
		// uart1_log<64>(LOG_LEVEL::FATAL, "freertos", "Stack Overflow!");
    const char msg[] = "Stack Overflow!";
    HAL_UART_Transmit(&huart1, reinterpret_cast<uint8_t*>(const_cast<char*>(msg)), strlen(msg), -1);
		vTaskDelay(500);
	}
}

void vApplicationMallocFailedHook(void)
{
	for(;;)
	{
		// uart1_log<64>(LOG_LEVEL::FATAL, "freertos", "Malloc Failed!");
    const char msg[] = "Malloc Failed!";
    HAL_UART_Transmit(&huart1, reinterpret_cast<uint8_t*>(const_cast<char*>(msg)), strlen(msg), -1);
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

void handle_config_assert(const char* file, const int line, const char* msg)
{
  Stack_string<128> str;
  str.sprintf("Config assert at %s:%d: %d", file, line, msg);
  {
    const char msg[] = "Config Assert - ";
    HAL_UART_Transmit(&huart1, reinterpret_cast<uint8_t*>(const_cast<char*>(str.data())), str.size(), -1);
  }
}

}