#include "stm32h7xx_it.h"

#include "main.h"
#include "hal_inst.h"

#include "cmsis_os.h"
#include "core_cm7.h"

extern PCD_HandleTypeDef hpcd_USB_OTG_HS;
extern TIM_HandleTypeDef htim17;

void FDCAN1_IT0_IRQHandler(void);
void FDCAN2_IT0_IRQHandler(void);
void FDCAN1_IT1_IRQHandler(void);
void FDCAN2_IT1_IRQHandler(void);

void NMI_Handler(void)
{

}

void HardFault_Handler(void)
{
  // Configurable Fault Status Register
  volatile uint32_t CFSR = SCB->CFSR;
  // Hard Fault Status Register
  volatile uint32_t HFSR = SCB->HFSR;
  // Debug Fault Status Register
  volatile uint32_t DFSR = SCB->DFSR;
  // Auxiliary Fault Status Register
  volatile uint32_t AFSR = SCB->AFSR;
  // MemManage Fault Address Register
  volatile uint32_t MMFAR = SCB->MMFAR;
  // BusFault Address Register
  volatile uint32_t BFAR = SCB->BFAR;
  //System Handler Control and State Register
  volatile uint32_t SHCSR = SCB->SHCSR;
  
  for(;;)
  {

  }
}

void MemManage_Handler(void)
{
  // Configurable Fault Status Register
  volatile uint32_t CFSR = SCB->CFSR;
  // Hard Fault Status Register
  volatile uint32_t HFSR = SCB->HFSR;
  // Debug Fault Status Register
  volatile uint32_t DFSR = SCB->DFSR;
  // Auxiliary Fault Status Register
  volatile uint32_t AFSR = SCB->AFSR;
  // MemManage Fault Address Register
  volatile uint32_t MMFAR = SCB->MMFAR;
  // BusFault Address Register
  volatile uint32_t BFAR = SCB->BFAR;
  //System Handler Control and State Register
  volatile uint32_t SHCSR = SCB->SHCSR;

  for(;;)
  {

  }
}

void BusFault_Handler(void)
{
  // Configurable Fault Status Register
  volatile uint32_t CFSR = SCB->CFSR;
  // Hard Fault Status Register
  volatile uint32_t HFSR = SCB->HFSR;
  // Debug Fault Status Register
  volatile uint32_t DFSR = SCB->DFSR;
  // Auxiliary Fault Status Register
  volatile uint32_t AFSR = SCB->AFSR;
  // MemManage Fault Address Register
  volatile uint32_t MMFAR = SCB->MMFAR;
  // BusFault Address Register
  volatile uint32_t BFAR = SCB->BFAR;
  //System Handler Control and State Register
  volatile uint32_t SHCSR = SCB->SHCSR;

  for(;;)
  {

  }
}

void UsageFault_Handler(void)
{
  // Configurable Fault Status Register
  volatile uint32_t CFSR = SCB->CFSR;
  // Hard Fault Status Register
  volatile uint32_t HFSR = SCB->HFSR;
  // Debug Fault Status Register
  volatile uint32_t DFSR = SCB->DFSR;
  // Auxiliary Fault Status Register
  volatile uint32_t AFSR = SCB->AFSR;
  // MemManage Fault Address Register
  volatile uint32_t MMFAR = SCB->MMFAR;
  // BusFault Address Register
  volatile uint32_t BFAR = SCB->BFAR;
  //System Handler Control and State Register
  volatile uint32_t SHCSR = SCB->SHCSR;

  for(;;)
  {

  }
}

void DebugMon_Handler(void)
{

}

void FDCAN1_IT0_IRQHandler(void)
{
  HAL_FDCAN_IRQHandler(&hfdcan1);
}

void FDCAN1_IT1_IRQHandler(void)
{
  HAL_FDCAN_IRQHandler(&hfdcan1);
}

void FDCAN_CAL_IRQHandler(void)
{
  HAL_FDCAN_IRQHandler(&hfdcan1);
}

void QUADSPI_IRQHandler(void)
{
  HAL_QSPI_IRQHandler(&hqspi);
}

void TIM17_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&htim17);
}
