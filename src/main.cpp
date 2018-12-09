
#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"
//#include "stm32h7xx_hal_def.h"
//#include "stm32h7xx_hal_pwr.h"

#include "FreeRTOS.h"
#include "task.h"

#include <array>
#include <algorithm>

void Error_Handler();
void SystemClock_Config();

void SystemClock_Config()
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /**Configure the main internal regulator output voltage 
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  while ((PWR->D3CR & (PWR_D3CR_VOSRDY)) != PWR_D3CR_VOSRDY) 
  {
    
  }
  /**Macro to configure the PLL clock source 
  */
  __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);
  /**Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 2;
  RCC_OscInitStruct.PLL.PLLN = 32;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 5;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /**Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_FDCAN|RCC_PERIPHCLK_USART1;
  PeriphClkInitStruct.PLL2.PLL2M = 2;
  PeriphClkInitStruct.PLL2.PLL2N = 16;
  PeriphClkInitStruct.PLL2.PLL2P = 2;
  PeriphClkInitStruct.PLL2.PLL2Q = 2;
  PeriphClkInitStruct.PLL2.PLL2R = 2;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  PeriphClkInitStruct.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL2;
  PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16CLKSOURCE_D2PCLK2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

void Error_Handler()
{

}

#define ULPI_CLK_EN_Pin GPIO_PIN_1
#define ULPI_CLK_EN_GPIO_Port GPIOA
#define ULPI_nRESET_Pin GPIO_PIN_2
#define ULPI_nRESET_GPIO_Port GPIOA

#define CAN_SILENT_Pin GPIO_PIN_14
#define CAN_SILENT_GPIO_Port GPIOB
#define CAN_STDBY_Pin GPIO_PIN_15
#define CAN_STDBY_GPIO_Port GPIOB

#define RED1_Pin GPIO_PIN_12
#define RED1_GPIO_Port GPIOD
#define GREEN1_Pin GPIO_PIN_13
#define GREEN1_GPIO_Port GPIOD
#define RED2_Pin GPIO_PIN_14
#define RED2_GPIO_Port GPIOD
#define GREEN2_Pin GPIO_PIN_15
#define GREEN2_GPIO_Port GPIOD

void task1(void* ctx)
{
  for(;;)
  {
    HAL_GPIO_TogglePin(GPIOD, RED1_Pin);
    vTaskDelay(pdMS_TO_TICKS(500));
    HAL_GPIO_TogglePin(GPIOD, RED1_Pin);

    HAL_GPIO_TogglePin(GPIOD, GREEN1_Pin);
    vTaskDelay(pdMS_TO_TICKS(500));
    HAL_GPIO_TogglePin(GPIOD, GREEN1_Pin);
  }
}

void task2(void* ctx)
{
  for(;;)
  {
    HAL_GPIO_TogglePin(GPIOD, RED2_Pin);
    vTaskDelay(pdMS_TO_TICKS(500));
    HAL_GPIO_TogglePin(GPIOD, RED2_Pin);
    
    HAL_GPIO_TogglePin(GPIOD, GREEN2_Pin);
    vTaskDelay(pdMS_TO_TICKS(500));
    HAL_GPIO_TogglePin(GPIOD, GREEN2_Pin);
  }
}

extern "C"
{
void vApplicationIdleHook( void )
{
  HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
}

void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
  for(;;)
  {

  }
}

void vApplicationMallocFailedHook(void)
{
  for(;;)
  {
    
  }
}
}

void read_des(std::array<uint32_t, 3>* const out_uid)
{
  const uint32_t* uid_base = reinterpret_cast<const uint32_t*>(0x1FF1E800);
  std::copy_n(uid_base, 3, out_uid->data());
}

void read_des(uint32_t* const out_flash_size)
{
  const uint32_t* flash_size_base = reinterpret_cast<const uint32_t*>(0x1FF1E880);

  *out_flash_size = *flash_size_base;
}

int main()
{
	SCB_EnableICache();
	SCB_EnableDCache();

	HAL_Init();

  __HAL_RCC_SYSCFG_CLK_ENABLE();

  /* System interrupt init*/
  /* PendSV_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(PendSV_IRQn, 15, 0);

  /* Disable the Internal Voltage Reference buffer */
  HAL_SYSCFG_DisableVREFBUF();

  /* Configure the internal voltage reference buffer high impedance mode */
  HAL_SYSCFG_VREFBUF_HighImpedanceConfig(SYSCFG_VREFBUF_HIGH_IMPEDANCE_ENABLE);

  //Configure flash for 100MHz AXI / Vcore VOS3
  //Could be reduced to 1 at VOS2 or VOS1
  //WRHIGHFREQ defaults to 0b11
  //FLASH_SetLatency(FLASH_LATENCY_2);

	SystemClock_Config();
  SystemCoreClock = 200000000U;

	__HAL_RCC_CRC_CLK_ENABLE();
	__HAL_RCC_HASH_CLK_ENABLE();

	__HAL_RCC_FDCAN_CLK_ENABLE();
	__HAL_RCC_USART1_CLK_ENABLE();

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();

	//USART1 - PA9 / PA10
  if(0)
	{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);		
	}


	//FDCAN1 - PA11 / PA12
  if(0)
	{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	}

	//ULPI_CLK_EN_Pin, ULPI_nRESET_Pin
  //if(0)
	{
	HAL_GPIO_WritePin(GPIOA, ULPI_CLK_EN_Pin|ULPI_nRESET_Pin, GPIO_PIN_RESET);

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = ULPI_CLK_EN_Pin|ULPI_nRESET_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	}

	//CAN_SILENT_Pin, CAN_STDBY_Pin 
  if(0)
	{
	HAL_GPIO_WritePin(GPIOB, CAN_SILENT_Pin|CAN_STDBY_Pin, GPIO_PIN_RESET);

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = CAN_SILENT_Pin|CAN_STDBY_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	}

	//RED1_Pin GREEN1_Pin RED2_Pin GREEN2_Pin
  //if(0)
	{
	HAL_GPIO_WritePin(GPIOD, RED1_Pin|GREEN1_Pin|RED2_Pin|GREEN2_Pin, GPIO_PIN_RESET);
	
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = RED1_Pin|GREEN1_Pin|RED2_Pin|GREEN2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	}

	//Start ULPI CLK
	HAL_GPIO_WritePin(GPIOA, ULPI_CLK_EN_Pin, GPIO_PIN_SET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(GPIOA, ULPI_nRESET_Pin, GPIO_PIN_SET);
  HAL_Delay(1);

  TaskHandle_t t1_handle = nullptr;
  BaseType_t ret1 = xTaskCreate(task1, "task1", 1024, nullptr, 1, &t1_handle);

  TaskHandle_t t2_handle = nullptr;
  BaseType_t ret2 = xTaskCreate(task2, "task2", 1024, nullptr, 1, &t2_handle);

  vTaskStartScheduler();

	return 0;
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM17 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM17) {
    HAL_IncTick();
    // xPortSysTickHandler();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}