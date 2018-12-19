#include "main.h"
#include "cmsis_os.h"
#include "usb_device.h"

void task1(void* ctx)
{
  MX_USB_DEVICE_Init();

  for(;;)
  {
    vTaskDelay(500);
  }
}

void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_USART1_UART_Init(void);
void MX_FDCAN1_Init(void);
void MX_CRC_Init(void);
void MX_HASH_Init(void);
void MX_RTC_Init(void);
void MX_RNG_Init(void);

int main(void)
{
  SCB_EnableICache();

  SCB_EnableDCache();

  HAL_Init();


  SystemClock_Config();

  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_FDCAN1_Init();
  MX_CRC_Init();
  MX_HASH_Init();
  MX_RTC_Init();
  MX_RNG_Init();

  TaskHandle_t task1Hand = nullptr;
  xTaskCreate(task1, "task1", 256, nullptr, 1, &task1Hand);
  vTaskStartScheduler();
  
  for(;;)
  {

  }
}