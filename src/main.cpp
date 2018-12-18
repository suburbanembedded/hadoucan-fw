
#include "usb_device.h"

#include "freertos_util/Task_heap.hpp"
#include "freertos_util/Task_static.hpp"
#include "freertos_util/Queue_static.hpp"
#include "freertos_util/Queue_static_pod.hpp"
#include "freertos_util/object_pool/Object_pool.hpp"

#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_flash_ex.h"
//#include "stm32h7xx_hal_def.h"
//#include "stm32h7xx_hal_pwr.h"

#include "FreeRTOS.h"
#include "task.h"

#include <array>
#include <algorithm>
#include <cstdio>
#include <cstdarg>
#include <cstdarg>
#include <algorithm>

extern "C"
{
  void Error_Handler();
}

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

  // if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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

#define ULPI_CLK_EN_Pin GPIO_PIN_0
#define ULPI_CLK_EN_GPIO_Port GPIOA
#define ULPI_nRESET_Pin GPIO_PIN_1
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

UART_HandleTypeDef console_uart;

template<size_t LEN>
size_t console_print(const char* fmt, ...)
{
	std::array<char, LEN> buf;

	va_list args;
	va_start (args, fmt);
	int ret = vsnprintf(buf.data(), buf.size(), fmt, args);
	va_end (args);

	if(ret < 0)
	{
		//error
		return 0;
	}
	else if(size_t(ret) >= buf.size())
	{
		//output was truncated
	}

	const size_t num_to_write = std::min<size_t>(buf.size() - 1, ret);

	HAL_UART_Transmit(&console_uart, (uint8_t*)buf.data(), num_to_write, HAL_MAX_DELAY);

	return num_to_write;
}

class task1 : public Task_heap
{
public:

  void work() override
  {

#if 0
    Queue_static_pod<int, 10> q;

    Object_pool<int, 10> op;

    int* a = op.allocate(5);
    *a = 5;
    volatile int b = *a;
    *a = b+1;
    op.deallocate(a);
    a = nullptr;

    {
      Object_pool<int, 10>::unique_node_ptr b = op.allocate_unique(6);
    }
#endif
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
};
task1 task1_instance __attribute__(( section(".ram_d1_noload_area") ));

class task2 : public Task_static<1024>
{
public:

  void work() override
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
};
task2 task2_instance __attribute__(( section(".ram_d1_noload_area") ));

class task3 : public Task_static<1024>
{
public:

  task3()
  {
    
  }

  void work() override
  {
	//Start ULPI CLK
	HAL_GPIO_WritePin(GPIOA, ULPI_CLK_EN_Pin, GPIO_PIN_SET);
	vTaskDelay(pdMS_TO_TICKS(10));
	// //Release ULPI from RESET
	HAL_GPIO_WritePin(GPIOA, ULPI_nRESET_Pin, GPIO_PIN_SET);
	vTaskDelay(pdMS_TO_TICKS(5));

    MX_USB_DEVICE_Init();
    
  	HAL_PWREx_EnableUSBVoltageDetector();

    for(;;)
    {
      HAL_GPIO_TogglePin(GPIOD, GREEN1_Pin);
      vTaskDelay(pdMS_TO_TICKS(500));
    }
  }
protected:
  UART_HandleTypeDef m_uart;
};
task3 task3_instance __attribute__(( section(".ram_d1_noload_area") ));

class task4 : public Task_static<1024>
{
public:

  task4()
  {
    m_fdcan = FDCAN_HandleTypeDef();
  }

  void work() override
  {
  	console_print<64>("CAN init startup\r\n");

    m_fdcan.Instance        = FDCAN1;
    // m_fdcan.ttcan
    m_fdcan.Init.FrameFormat = FDCAN_FRAME_FD_NO_BRS;
    m_fdcan.Init.Mode = FDCAN_MODE_NORMAL;
    // m_fdcan.Init.AutoRetransmission = DISABLE;
    m_fdcan.Init.AutoRetransmission = ENABLE;
    m_fdcan.Init.TransmitPause = DISABLE;
    m_fdcan.Init.ProtocolException = DISABLE;
    m_fdcan.Init.NominalPrescaler = 24;
    m_fdcan.Init.NominalSyncJumpWidth = 1;
    m_fdcan.Init.NominalTimeSeg1 = 13;
    m_fdcan.Init.NominalTimeSeg2 = 3;
    m_fdcan.Init.DataPrescaler = 24;
    m_fdcan.Init.DataSyncJumpWidth = 1;
    m_fdcan.Init.DataTimeSeg1 = 13;
    m_fdcan.Init.DataTimeSeg2 = 3;
    m_fdcan.Init.MessageRAMOffset = 0;
    m_fdcan.Init.StdFiltersNbr = 1;
    m_fdcan.Init.ExtFiltersNbr = 0;
    m_fdcan.Init.RxFifo0ElmtsNbr = 2;
    m_fdcan.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8;
    m_fdcan.Init.RxFifo1ElmtsNbr = 0;
    m_fdcan.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_8;
    m_fdcan.Init.RxBuffersNbr = 0;
    m_fdcan.Init.RxBufferSize = FDCAN_DATA_BYTES_8;
    m_fdcan.Init.TxEventsNbr = 0;
    m_fdcan.Init.TxBuffersNbr = 0;
    m_fdcan.Init.TxFifoQueueElmtsNbr = 2;
    m_fdcan.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
    m_fdcan.Init.TxElmtSize = FDCAN_DATA_BYTES_8;

	//0x4000AC00
	//0x4000D3FF
	//0x0000 - 0x27FF = 10240 bytes = 2560 words
	//regions must be 4b aligned
    //m_fdcan.msgRam.StandardFilterSA = 0;//11 bit filter, 0 - 128 elements / 0 - 512 byte
    //m_fdcan.msgRam.ExtendedFilterSA = 0;//29 bit filter, 0 - 64 elements / 0 - 512 byte
    //m_fdcan.msgRam.RxFIFO0SA = 0;//rx fifo 0, 0 - 64 elements / 0 - 4608 byte
    //m_fdcan.msgRam.RxFIFO1SA = 0;//rx fifo 0, 0 - 64 elements / 0 - 4608 byte
    //m_fdcan.msgRam.RxBufferSA = 0;//rx buffer, 0 - 64 elements / 0 - 4608 byte
    //m_fdcan.msgRam.TxEventFIFOSA = 0;//tx event fifo, 0 - 32 elements / 0 - 256 byte
    //m_fdcan.msgRam.TxBufferSA = 0;//tx buffer, 0 - 32 elements / 0 - 2304 byte
    //m_fdcan.msgRam.TxFIFOQSA = 0;//tx fifo queue start address
    //m_fdcan.msgRam.TTMemorySA = 0;//trigger memory, 0 - 64 elements / 0 - 512 byte
    //m_fdcan.msgRam.EndAddress = 0;//msg ram end address

    m_fdcan.ErrorCode = 0;
    // m_fdcan.msgRam
    // m_fdcan.State

	if(HAL_FDCAN_Init(&m_fdcan) != HAL_OK)
	{
		console_print<64>("HAL_FDCAN_Init failed\r\n");
        for(;;)
        {
          vTaskDelay(pdMS_TO_TICKS(500));
        }
	}

	//rx filter, all in rxfifo0
    m_std_filt.IdType = FDCAN_STANDARD_ID;
    m_std_filt.FilterIndex = 0;
    m_std_filt.FilterType = FDCAN_FILTER_MASK;
    m_std_filt.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    m_std_filt.FilterID1 = 0x000;
    m_std_filt.FilterID2 = 0x000; /* For acceptance, MessageID and FilterID1 must match exactly */
    HAL_FDCAN_ConfigFilter(&m_fdcan, &m_std_filt);

    //set watermark
	HAL_FDCAN_ConfigFifoWatermark(&m_fdcan, FDCAN_CFG_RX_FIFO0, 2);

	//enable watermark notification
	HAL_FDCAN_ActivateNotification(&m_fdcan, FDCAN_IT_RX_FIFO0_WATERMARK, 0);

	if(HAL_FDCAN_Start(&m_fdcan) != HAL_OK)
	{
		console_print<64>("HAL_FDCAN_Start failed\r\n");
        for(;;)
        {
          vTaskDelay(pdMS_TO_TICKS(500));
        }
	}

    for(;;)
    {
		FDCAN_TxHeaderTypeDef tx_header;

        // tx_header.Identifier = 0x111;
        // tx_header.IdType = FDCAN_STANDARD_ID;
        tx_header.Identifier = 0xAB111;
        tx_header.IdType = FDCAN_EXTENDED_ID;
        tx_header.TxFrameType = FDCAN_DATA_FRAME;
        tx_header.DataLength = FDCAN_DLC_BYTES_8;
        tx_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
        tx_header.BitRateSwitch = FDCAN_BRS_OFF;
        tx_header.FDFormat = FDCAN_CLASSIC_CAN;
        // tx_header.FDFormat = FDCAN_FD_CAN;
        tx_header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
        tx_header.MessageMarker = 0;

        std::array<uint8_t, 8> tx_data = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};

		if(HAL_FDCAN_AddMessageToTxFifoQ(&m_fdcan, &tx_header, tx_data.data()) == HAL_OK)
		{
			console_print<128>("HAL_FDCAN_AddMessageToTxFifoQ ok\r\n");
		}
		else
		{
			console_print<128>("HAL_FDCAN_AddMessageToTxFifoQ failed\r\n");
	        for(;;)
	        {
	          vTaskDelay(pdMS_TO_TICKS(500));
	        }
		}


        vTaskDelay(pdMS_TO_TICKS(500));
    }
  }
protected:
  FDCAN_HandleTypeDef m_fdcan;
  FDCAN_FilterTypeDef m_std_filt;
};
task4 task4_instance __attribute__(( section(".ram_d1_noload_area") ));

extern "C"
{

  uint8_t ucHeap[ configTOTAL_HEAP_SIZE ] __attribute__(( section(".ram_d1_noload_area") ));
  uint8_t ucHeap2[ 128U * 1024U ]         __attribute__(( section(".ram_d2_noload_area") ));
  uint8_t ucHeap3[ 64U * 1024U ]          __attribute__(( section(".ram_d3_noload_area") ));

  StaticTask_t xIdleTaskTCB;
  StackType_t  uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
  {
    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  }

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

void read_flash_size(uint32_t* const out_flash_size)
{
  const uint32_t* flash_size_base = reinterpret_cast<const uint32_t*>(0x1FF1E880);

  *out_flash_size = *flash_size_base;
}

void set_gpio_low_power(GPIO_TypeDef* const gpio)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_All;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  // GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(gpio, &GPIO_InitStruct);
}

void set_gpio_low_power()
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOJ_CLK_ENABLE();
  __HAL_RCC_GPIOK_CLK_ENABLE();
  
  set_gpio_low_power(GPIOA);
  set_gpio_low_power(GPIOB);
  set_gpio_low_power(GPIOC);
  set_gpio_low_power(GPIOD);
  set_gpio_low_power(GPIOE);
  set_gpio_low_power(GPIOF);
  set_gpio_low_power(GPIOG);
  set_gpio_low_power(GPIOH);
  set_gpio_low_power(GPIOI);
  set_gpio_low_power(GPIOJ);
  set_gpio_low_power(GPIOK);

  __HAL_RCC_GPIOA_CLK_DISABLE();
  __HAL_RCC_GPIOB_CLK_DISABLE();
  __HAL_RCC_GPIOC_CLK_DISABLE();
  __HAL_RCC_GPIOD_CLK_DISABLE();
  __HAL_RCC_GPIOE_CLK_DISABLE();
  __HAL_RCC_GPIOF_CLK_DISABLE();
  __HAL_RCC_GPIOG_CLK_DISABLE();
  __HAL_RCC_GPIOH_CLK_DISABLE();
  __HAL_RCC_GPIOI_CLK_DISABLE();
  __HAL_RCC_GPIOJ_CLK_DISABLE();
  __HAL_RCC_GPIOK_CLK_DISABLE();
}

int main()
{
  ucHeap[0]  = 0;
  ucHeap2[0] = 0;
  ucHeap3[0] = 0;

  SCB_EnableICache();
  SCB_EnableDCache();

  HAL_Init();

  //set_gpio_low_power();

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();

  __HAL_RCC_SYSCFG_CLK_ENABLE();

  /* System interrupt init*/
  /* PendSV_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(PendSV_IRQn, 15, 0);

  /* Disable the Internal Voltage Reference buffer */
  HAL_SYSCFG_DisableVREFBUF();

  /* Configure the internal voltage reference buffer high impedance mode */
  HAL_SYSCFG_VREFBUF_HighImpedanceConfig(SYSCFG_VREFBUF_HIGH_IMPEDANCE_ENABLE);

  //Configure flash for 100MHz AXI / Vcore VOS3
  //TODO: CUBE MIGHT BE DOING THIS WRONG
  //Could be reduced to 1 at VOS2 or VOS1
  // __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  //WRHIGHFREQ defaults to 0b11
  // __HAL_FLASH_SET_LATENCY(FLASH_LATENCY_2);

  SystemClock_Config();
  SystemCoreClock = 200000000U;

  __HAL_RCC_CRC_CLK_ENABLE();
  __HAL_RCC_HASH_CLK_ENABLE();

  __HAL_RCC_USART1_CLK_ENABLE();
  __HAL_RCC_FDCAN_CLK_ENABLE();

  //disconnect internal analog switches
  MODIFY_REG(SYSCFG->PMCR, 
    SYSCFG_PMCR_PA0SO_Msk | SYSCFG_PMCR_PA1SO_Msk | SYSCFG_PMCR_PC2SO_Msk | SYSCFG_PMCR_PC3SO_Msk, 
    SYSCFG_PMCR_PA0SO | SYSCFG_PMCR_PA1SO | SYSCFG_PMCR_PC2SO | SYSCFG_PMCR_PC3SO
  );

  //USART1 - PA9 / PA10
  // if(0)
  {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);    
  }


  //FDCAN1 - PA11 / PA12
  // if(0)
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
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  }

  //CAN_SILENT_Pin, CAN_STDBY_Pin 
  {
  HAL_GPIO_WritePin(GPIOB, CAN_SILENT_Pin|CAN_STDBY_Pin, GPIO_PIN_SET);

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

  //Enable CAN
  HAL_GPIO_WritePin(GPIOB, CAN_STDBY_Pin, GPIO_PIN_RESET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(GPIOB, CAN_SILENT_Pin, GPIO_PIN_RESET);
  HAL_Delay(1);

console_uart = UART_HandleTypeDef();
console_uart.Instance        = USART1;
console_uart.Init.BaudRate   = 115200;
console_uart.Init.WordLength = UART_WORDLENGTH_8B;
console_uart.Init.StopBits   = UART_STOPBITS_1;
console_uart.Init.Parity     = UART_PARITY_NONE;
console_uart.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
console_uart.Init.Mode       = UART_MODE_TX_RX;

	if(HAL_UART_Init(&console_uart) != HAL_OK)
	{
	  for(;;)
	  {
	    HAL_Delay(pdMS_TO_TICKS(500));
	  }
	}

	console_print<64>("Startup\r\n");


  // task1_instance.launch("task1", 1024, 1);
  // task2_instance.launch("task2", 1);
  task3_instance.launch("task3", 2);
  task4_instance.launch("task4", 1);

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

extern "C"
{
  void Error_Handler();
  
  void Error_Handler(void)
  {
    for(;;)
    {
      HAL_GPIO_TogglePin(RED1_GPIO_Port, RED1_Pin);
      HAL_Delay(500);
    }
  }
}