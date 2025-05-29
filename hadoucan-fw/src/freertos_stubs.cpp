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

// 0 == thread mode
// else, active isr
uint8_t get_active_isr()
{
  // Interrupt program status register
  uint32_t IPSR;
  __asm__ volatile (
    "MRS %0, IPSR"
    : "=r" (IPSR)
    : 
    : "memory"
  );

  const uint32_t ISR_NUMBER = (IPSR & 0x000000FFUL) >> 0;

  return ISR_NUMBER;
}

bool is_isr_active()
{
  return get_active_isr() != 0;
}

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

   // Disable ISR and sync
  __asm__ volatile (
    "cpsid i\n"
    "isb\n"
    "dsb\n"
    : 
    : 
    : "memory"
  );

  // Only enabled ISR or events cause wake
  // Clear deep sleep register, sleep normal
  // Do not sleep on return to thread mode
  CLEAR_BIT (SCB->SCR, SCB_SCR_SEVONPEND_Msk | SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk);

  // sync SCB write, WFI, sync/reload pipeline, enable ISR, sync/reload pipeline
  // certain platforms can crash on complex wfi return if no isb after wfi (arm core bug? - https://cliffle.com/blog/stm32-wfi-bug/)
  __asm__ volatile (
    "dsb\n"
    "wfi\n"
    "isb\n"
    "cpsie i\n"
    "isb\n"
    : 
    : 
    : "memory"
  );
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
    HAL_UART_Transmit(&huart1, reinterpret_cast<uint8_t*>(const_cast<char*>(str.data())), str.size(), -1);
  }
}

}