
#include "main.h"
#include "stm32h7xx_it.h"
#include "cmsis_os.h"

extern PCD_HandleTypeDef hpcd_USB_OTG_HS;
extern TIM_HandleTypeDef htim17;
extern FDCAN_HandleTypeDef hfdcan1;

void NMI_Handler(void)
{

}

void HardFault_Handler(void)
{
  for(;;)
  {

  }
}

void MemManage_Handler(void)
{
  for(;;)
  {

  }
}

void BusFault_Handler(void)
{
  for(;;)
  {

  }
}

void UsageFault_Handler(void)
{
  for(;;)
  {

  }
}

void DebugMon_Handler(void)
{

}

void OTG_HS_IRQHandler(void)
{
  HAL_PCD_IRQHandler(&hpcd_USB_OTG_HS);
}

void TIM17_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&htim17);
}

void FDCANx_IRQHandler(void)
{
  HAL_FDCAN_IRQHandler(&hfdcan1);
}
