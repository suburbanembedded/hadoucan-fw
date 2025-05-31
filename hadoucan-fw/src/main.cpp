#include "main.h"
#include "cmsis_os.h"

#include "lawicel/Lawicel_parser_stm32.hpp"

#include "tasks/USB_lawicel_task.hpp"
#include "tasks/LED_task.hpp"
#include "tasks/Timesync_task.hpp"
#include "tasks/STM32_fdcan_rx.hpp"
#include "tasks/Task_instances.hpp"

#include "tasks/USB_rx_buffer_task.hpp"
#include "tasks/USB_tx_buffer_task.hpp"

#include "freertos_cpp_util/Task_static.hpp"
#include "freertos_cpp_util/BSema_static.hpp"
#include "freertos_cpp_util/Mutex_static.hpp"
#include "freertos_cpp_util/Condition_variable.hpp"
#include "freertos_cpp_util/object_pool/Object_pool.hpp"

#include "common_util/Byte_util.hpp"

#include "W25Q16JV.hpp"
#include "W25Q16JV_conf_region.hpp"

#include "../external/tinyxml2/tinyxml2.h"

#include <memory>
#include <sstream>

#include <cstdio>
#include <cinttypes>

/*
class TinyXML_inc_printer : public tinyxml2::XMLVisitor
{
public:
	TinyXML_inc_printer()
	{
		indent_level = 0;
	}

	bool VisitEnter(const tinyxml2::XMLDocument& doc) override
	{
		indent_level = 0;
		return true;
	}
	bool VisitExit(const tinyxml2::XMLDocument& doc) override
	{
		return true;
	}
	bool VisitEnter(const tinyxml2::XMLElement& ele, const tinyxml2::XMLAttribute* attr) override
	{
		print_indent();

		uart1_printf<64>("<%s", ele.Name());
		if(attr)
		{
			tinyxml2::XMLAttribute const* node = attr;
			
			do
			{
				uart1_printf<64>(" %s=\"%s\"", node->Name(), node->Value());
				node = node->Next();
			} while(node);
		}

		if(ele.NoChildren())
		{
			uart1_printf<64>("/>\n");
		}
		else
		{
			uart1_printf<64>(">");

			if(ele.FirstChild()->ToText() == nullptr)
			{
				indent_level++;

				uart1_printf<64>("\n");
			}
		}

		return true;
	}
	bool VisitExit(const tinyxml2::XMLElement& ele) override
	{
		if(ele.NoChildren())
		{
			return true;
		}

		if(ele.Parent() != nullptr)
		{
			if(ele.FirstChild()->ToText() == nullptr)
			{
				indent_level--;
				print_indent();
				uart1_printf<64>("</%s>\n", ele.Name());
			}
			else
			{
				uart1_printf<64>("</%s>\n", ele.Name());
			}
		}
		else
		{
			print_indent();
			uart1_printf<64>("</%s>\n", ele.Name());	
		}

		return true;
	}
	bool Visit(const tinyxml2::XMLDeclaration& decl) override
	{
		print_indent();
		uart1_printf<64>("<?%s?>\n", decl.Value());
		return true;
	}
	bool Visit(const tinyxml2::XMLText& text) override
	{
		uart1_printf<64>("%s", text.Value());
		return true;
	}
	bool Visit(const tinyxml2::XMLComment& com) override
	{
		print_indent();
		uart1_printf<64>("<!--%s-->\n", com.Value());
		return true;
	}
	bool Visit(const tinyxml2::XMLUnknown& unk) override
	{
		return true;
	}
protected:

	void print_indent()
	{
		for(size_t i = 0 ; i < indent_level; i++)
		{
			uart1_printf<64>("\t");
		}
	}

	size_t indent_level;
};
*/

void set_gpio_low_power(GPIO_TypeDef* const gpio)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_PIN_All;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	// GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(gpio, &GPIO_InitStruct);
}

void set_all_gpio_low_power()
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

int main(void)
{
	//confg mpu
	if(1)
	{
		/*
		ITCMRAM, 0x00000000, 64K

		FLASH, 0x08000000, 128K

		DTCMRAM, 0x20000000, 128K

		AXI_D1_SRAM, 0x24000000, 512K,  CPU Inst/Data

		AHB_D2_SRAM1, 0x30000000, 128K, CPU Inst
		AHB_D2_SRAM2, 0x30020000, 128K, CPU Data
		AHB_D2_SRAM3, 0x30040000, 32K,  Peripheral Buffers

		AHB_D3_SRAM4, 0x38000000, 64K

		BBRAM, 0x38800000, 4K

		QUADSPI, 0x90000000, 16M

		Peripherals, 0x40000000, 512M
		*/

		MPU_Region_InitTypeDef mpu_reg;
		
		HAL_MPU_Disable();

		/*
		// Global
		// Normal, no access
		mpu_reg.Enable = MPU_REGION_ENABLE;
		mpu_reg.Number = MPU_REGION_NUMBER0;
		mpu_reg.BaseAddress = 0x00000000;
		mpu_reg.Size = MPU_REGION_SIZE_4GB;
		mpu_reg.SubRegionDisable = 0x00;
		mpu_reg.AccessPermission = MPU_REGION_NO_ACCESS;
		mpu_reg.TypeExtField = MPU_TEX_LEVEL1;
		mpu_reg.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
		mpu_reg.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
		mpu_reg.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
		mpu_reg.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
		HAL_MPU_ConfigRegion(&mpu_reg);
		*/

		// ITCMRAM
		// Normal, Non-cacheable
		mpu_reg.Enable = MPU_REGION_ENABLE;
		mpu_reg.Number = MPU_REGION_NUMBER1;
		mpu_reg.BaseAddress = 0x00000000;
		mpu_reg.Size = MPU_REGION_SIZE_64KB;
		mpu_reg.SubRegionDisable = 0x00;
		mpu_reg.AccessPermission = MPU_REGION_FULL_ACCESS;
		mpu_reg.TypeExtField = MPU_TEX_LEVEL1;
		mpu_reg.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
		mpu_reg.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
		mpu_reg.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
		mpu_reg.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
		HAL_MPU_ConfigRegion(&mpu_reg);

		// FLASH
		// Outer and inner write-through. No Write-Allocate.
		mpu_reg.Enable = MPU_REGION_ENABLE;
		mpu_reg.Number = MPU_REGION_NUMBER2;
		mpu_reg.BaseAddress = 0x08000000;
		mpu_reg.Size = MPU_REGION_SIZE_128KB;
		mpu_reg.SubRegionDisable = 0x00;
		mpu_reg.AccessPermission = MPU_REGION_PRIV_RO_URO;
		mpu_reg.TypeExtField = MPU_TEX_LEVEL0;
		mpu_reg.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
		mpu_reg.IsCacheable = MPU_ACCESS_CACHEABLE;
		mpu_reg.IsBufferable = MPU_ACCESS_BUFFERABLE;
		mpu_reg.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
		HAL_MPU_ConfigRegion(&mpu_reg);

		// DTCMRAM
		// Normal, Non-cacheable
		mpu_reg.Enable = MPU_REGION_ENABLE;
		mpu_reg.Number = MPU_REGION_NUMBER3;
		mpu_reg.BaseAddress = 0x20000000;
		mpu_reg.Size = MPU_REGION_SIZE_128KB;
		mpu_reg.SubRegionDisable = 0x00;
		mpu_reg.AccessPermission = MPU_REGION_FULL_ACCESS;
		mpu_reg.TypeExtField = MPU_TEX_LEVEL1;
		mpu_reg.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
		mpu_reg.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
		mpu_reg.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
		mpu_reg.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
		HAL_MPU_ConfigRegion(&mpu_reg);

		// AXI_D1_SRAM
		// Write-back, no write allocate
		mpu_reg.Enable = MPU_REGION_ENABLE;
		mpu_reg.Number = MPU_REGION_NUMBER4;
		mpu_reg.BaseAddress = 0x24000000;
		mpu_reg.Size = MPU_REGION_SIZE_512KB;
		mpu_reg.SubRegionDisable = 0x00;
		// mpu_reg.AccessPermission = MPU_REGION_FULL_ACCESS;
		mpu_reg.AccessPermission = MPU_REGION_PRIV_RO_URO;
		mpu_reg.TypeExtField = MPU_TEX_LEVEL0;
		mpu_reg.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
		mpu_reg.IsCacheable = MPU_ACCESS_CACHEABLE;
		mpu_reg.IsBufferable = MPU_ACCESS_BUFFERABLE;
		mpu_reg.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
		HAL_MPU_ConfigRegion(&mpu_reg);

		// AHB_D2_SRAM1
		// Write-back, no write allocate
		mpu_reg.Enable = MPU_REGION_ENABLE;
		mpu_reg.Number = MPU_REGION_NUMBER5;
		mpu_reg.BaseAddress = 0x30000000;
		mpu_reg.Size = MPU_REGION_SIZE_128KB;
		mpu_reg.SubRegionDisable = 0x00;
		mpu_reg.AccessPermission = MPU_REGION_FULL_ACCESS;
		mpu_reg.TypeExtField = MPU_TEX_LEVEL0;
		mpu_reg.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
		mpu_reg.IsCacheable = MPU_ACCESS_CACHEABLE;
		mpu_reg.IsBufferable = MPU_ACCESS_BUFFERABLE;
		mpu_reg.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
		HAL_MPU_ConfigRegion(&mpu_reg);

		// AHB_D2_SRAM2
		// Write-back, no write allocate
		mpu_reg.Enable = MPU_REGION_ENABLE;
		mpu_reg.Number = MPU_REGION_NUMBER6;
		mpu_reg.BaseAddress = 0x30020000;
		mpu_reg.Size = MPU_REGION_SIZE_128KB;
		mpu_reg.SubRegionDisable = 0x00;
		mpu_reg.AccessPermission = MPU_REGION_FULL_ACCESS;
		mpu_reg.TypeExtField = MPU_TEX_LEVEL0;
		mpu_reg.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
		mpu_reg.IsCacheable = MPU_ACCESS_CACHEABLE;
		mpu_reg.IsBufferable = MPU_ACCESS_BUFFERABLE;
		mpu_reg.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
		HAL_MPU_ConfigRegion(&mpu_reg);

		// AHB_D2_SRAM3
		// Normal, Non-cacheable
		mpu_reg.Enable = MPU_REGION_ENABLE;
		mpu_reg.Number = MPU_REGION_NUMBER7;
		mpu_reg.BaseAddress = 0x30040000;
		mpu_reg.Size = MPU_REGION_SIZE_32KB;
		mpu_reg.SubRegionDisable = 0x00;
		mpu_reg.AccessPermission = MPU_REGION_FULL_ACCESS;
		mpu_reg.TypeExtField = MPU_TEX_LEVEL1;
		mpu_reg.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
		mpu_reg.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
		mpu_reg.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
		mpu_reg.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
		HAL_MPU_ConfigRegion(&mpu_reg);

		// AHB_D3_SRAM4
		// Normal, Non-cacheable
		mpu_reg.Enable = MPU_REGION_ENABLE;
		mpu_reg.Number = MPU_REGION_NUMBER8;
		mpu_reg.BaseAddress = 0x38000000;
		mpu_reg.Size = MPU_REGION_SIZE_64KB;
		mpu_reg.SubRegionDisable = 0x00;
		mpu_reg.AccessPermission = MPU_REGION_FULL_ACCESS;
		mpu_reg.TypeExtField = MPU_TEX_LEVEL1;
		mpu_reg.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
		mpu_reg.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
		mpu_reg.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
		mpu_reg.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
		HAL_MPU_ConfigRegion(&mpu_reg);

		// BBSRAM
		// Write-back, no write allocate
		mpu_reg.Enable = MPU_REGION_ENABLE;
		mpu_reg.Number = MPU_REGION_NUMBER9;
		mpu_reg.BaseAddress = 0x38800000;
		mpu_reg.Size = MPU_REGION_SIZE_4KB;
		mpu_reg.SubRegionDisable = 0x00;
		mpu_reg.AccessPermission = MPU_REGION_FULL_ACCESS;
		mpu_reg.TypeExtField = MPU_TEX_LEVEL0;
		mpu_reg.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
		mpu_reg.IsCacheable = MPU_ACCESS_CACHEABLE;
		mpu_reg.IsBufferable = MPU_ACCESS_BUFFERABLE;
		mpu_reg.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
		HAL_MPU_ConfigRegion(&mpu_reg);

		// QUADSPI
		// Write through, no write allocate
		mpu_reg.Enable = MPU_REGION_ENABLE;
		mpu_reg.Number = MPU_REGION_NUMBER10;
		mpu_reg.BaseAddress = 0x90000000;
		mpu_reg.Size = MPU_REGION_SIZE_16MB;
		mpu_reg.SubRegionDisable = 0x00;
		mpu_reg.AccessPermission = MPU_REGION_FULL_ACCESS;
		mpu_reg.TypeExtField = MPU_TEX_LEVEL0;
		mpu_reg.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
		mpu_reg.IsCacheable = MPU_ACCESS_CACHEABLE;
		mpu_reg.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
		mpu_reg.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
		HAL_MPU_ConfigRegion(&mpu_reg);

		// Peripherals
		// Strongly Ordered
		/*
		mpu_reg.Enable = MPU_REGION_ENABLE;
		mpu_reg.Number = MPU_REGION_NUMBER11;
		mpu_reg.BaseAddress = 0x40000000;
		mpu_reg.Size = MPU_REGION_SIZE_512MB;
		mpu_reg.SubRegionDisable = 0x00;
		mpu_reg.AccessPermission = MPU_REGION_FULL_ACCESS;
		mpu_reg.TypeExtField = MPU_TEX_LEVEL0;
		mpu_reg.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
		mpu_reg.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
		mpu_reg.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
		mpu_reg.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
		HAL_MPU_ConfigRegion(&mpu_reg);
		*/
		// Non-shareable device 
		mpu_reg.Enable = MPU_REGION_ENABLE;
		mpu_reg.Number = MPU_REGION_NUMBER11;
		mpu_reg.BaseAddress = 0x40000000;
		mpu_reg.Size = MPU_REGION_SIZE_512MB;
		mpu_reg.SubRegionDisable = 0x00;
		mpu_reg.AccessPermission = MPU_REGION_FULL_ACCESS;
		mpu_reg.TypeExtField = MPU_TEX_LEVEL2;
		mpu_reg.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
		mpu_reg.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
		mpu_reg.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
		mpu_reg.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
		HAL_MPU_ConfigRegion(&mpu_reg);

		// Privledged code may use background mem map
		HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

		//No background mem map
		//MPU enabled during MMI
		// HAL_MPU_Enable(MPU_HARDFAULT_NMI);
		
		if(1)
		{
			SCB_EnableICache();
			SCB_EnableDCache();
		}
	}

	//enable core interrupts
	SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk | SCB_SHCSR_BUSFAULTENA_Msk | SCB_SHCSR_MEMFAULTENA_Msk;
	// SCB->CCR   &= ~(SCB_CCR_UNALIGN_TRP_Msk);
	// SCB->CCR   |= SCB_CCR_UNALIGN_TRP_Msk;

	HAL_Init();

	SystemClock_Config();

	//Enable ISR
	__set_BASEPRI(0);
	__ISB();

	//Enable backup domain in standby and Vbat mode
	HAL_PWREx_EnableBkUpReg();

	MX_GPIO_Init();
	MX_CRC_Init();
	MX_HASH_Init();
	MX_RNG_Init();
	MX_TIM3_Init();
	MX_USART1_UART_Init();
	MX_FDCAN1_Init();
	MX_QUADSPI_Init();
	MX_RTC_Init();

	// Wait 100ms for clock startup
	{
		const uint32_t t0 = HAL_GetTick();
		while((HAL_GetTick() - t0) < 100)
		{

		}

		HAL_GPIO_WritePin(GPIOA, ULPI_nRESET_Pin, GPIO_PIN_SET);
	}


	#if 0
	// Startup FDCAN clock and GPIOs
	if(0)
	{
		MX_FDCAN1_Init();
	}
	else
	{
		RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
	    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
	    PeriphClkInitStruct.PLL2.PLL2M = 2;
	    PeriphClkInitStruct.PLL2.PLL2N = 20;
	    PeriphClkInitStruct.PLL2.PLL2P = 2;
	    PeriphClkInitStruct.PLL2.PLL2Q = 4;
	    PeriphClkInitStruct.PLL2.PLL2R = 3;
	    PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
	    PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
	    PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
	    PeriphClkInitStruct.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL2;
	    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
	    {
	      Error_Handler();
	    }

	    /* Peripheral clock enable */
	    __HAL_RCC_FDCAN_CLK_ENABLE();

	    __HAL_RCC_GPIOA_CLK_ENABLE();
	    /**FDCAN1 GPIO Configuration
	    PA11     ------> FDCAN1_RX
	    PA12     ------> FDCAN1_TX
	    */
	    GPIO_InitTypeDef GPIO_InitStruct = {0};
	    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
	    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	    GPIO_InitStruct.Pull = GPIO_NOPULL;
	    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
	    GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
	    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	}
	#endif

	if(0)
	{
		/*Configure GPIO pin : PA8 */
		GPIO_InitTypeDef GPIO_InitStruct = {0};
		GPIO_InitStruct.Pin = GPIO_PIN_8;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
		GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSE, RCC_MCODIV_1);
	}

	//enable high speed for now
	//TODO: make this config
	if(1)
	{
		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();

		GPIO_InitTypeDef GPIO_InitStruct = {0};
		GPIO_InitStruct.Pin = CAN_SLOPE_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
		HAL_GPIO_WritePin(CAN_SLOPE_GPIO_Port, CAN_SLOPE_Pin, GPIO_PIN_RESET);
		HAL_GPIO_Init(CAN_SLOPE_GPIO_Port, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = CAN_SILENT_Pin|CAN_STDBY_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
		HAL_GPIO_WritePin(GPIOB, CAN_SILENT_Pin|CAN_STDBY_Pin, GPIO_PIN_RESET);
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	}

	main_task.launch("main_task", 15);

	vTaskStartScheduler();

	for(;;)
	{

	}

	return 0;
}
