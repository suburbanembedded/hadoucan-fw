#include "Main_task.hpp"

#include "global_app_inst.hpp"
#include "sw_ver.hpp"

#include "Task_instances.hpp"

#include "common_util/Byte_util.hpp"

#include "freertos_cpp_util/logging/Global_logger.hpp"

#include "tusb.h"

#include "hal_inst.h"
#include "stm32h7xx_hal.h"
#include "main.h"

#include <cinttypes>

using freertos_util::logging::LOG_LEVEL;

namespace
{
	bool can_rx_to_lawicel(const std::string& str)
	{
		return usb_lawicel_task.get_lawicel()->queue_rx_packet(str);
	}
}

void Main_task::work()
{
	{
		freertos_util::logging::Global_logger::set(&logging_task.get_logger());
		freertos_util::logging::Global_logger::get()->set_sev_mask_level(freertos_util::logging::LOG_LEVEL::INFO);
	}

	freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();

	{
		CAN_USB_app::get_unique_id_str(&usb_id_str);
		logger->log(LOG_LEVEL::INFO, "main", "Initialing");
		logger->log(LOG_LEVEL::INFO, "main", "CAN FD <-> USB Adapter");
		logger->log(LOG_LEVEL::INFO, "main", "P/N: SM-1301");
		logger->log(LOG_LEVEL::INFO, "main", "S/N: %s", usb_id_str.data());
		logger->log(LOG_LEVEL::INFO, "main", "Version: %d.%d.%d", SW_VER_MAJOR, SW_VER_MINOR, SW_VER_PATCH);
		logger->log(LOG_LEVEL::INFO, "main", "Commit: %s", GIT_COMMIT);
	}
	{
		const uint32_t idcode = DBGMCU->IDCODE;
		const uint16_t rev_id = (idcode & 0xFFFF0000) >> 16;
		const uint16_t dev_id = (idcode & 0x000007FF);

		if(dev_id == 0x450)
		{
			logger->log(LOG_LEVEL::INFO, "main", "Dev ID STM32H7xx (42, 43/53, 50)");
		}
		else
		{
			logger->log(LOG_LEVEL::WARN, "main", "Unk dev ID");
		}

		switch(rev_id)
		{
			case 0x1001:
			{
				logger->log(LOG_LEVEL::INFO, "main", "Silicon rev Z");
				logger->log(LOG_LEVEL::WARN, "main", "This silicon revision is not supported");
				break;
			}
			case 0x1003:
			{
				logger->log(LOG_LEVEL::INFO, "main", "Silicon rev Y");
				break;
			}
			case 0x2001:
			{
				logger->log(LOG_LEVEL::INFO, "main", "Silicon rev X");
				logger->log(LOG_LEVEL::WARN, "main", "This silicon revision is not supported");
				break;
			}
			case 0x2003:
			{
				logger->log(LOG_LEVEL::INFO, "main", "Silicon rev V");
				logger->log(LOG_LEVEL::WARN, "main", "This silicon revision is not supported, but will be soon");
				break;
			}
			default:
			{
				logger->log(LOG_LEVEL::WARN, "main", "Silicon rev unknown");
				logger->log(LOG_LEVEL::WARN, "main", "This silicon revision is not supported");
				break;
			}
		}
	}

	mount_fs();
	load_config();

	//load info
	CAN_USB_app_config::Config_Set config_struct;
	can_usb_app.get_config(&config_struct);
	CAN_USB_app_bitrate_table bitrate_table;
	can_usb_app.get_bitrate_tables(&bitrate_table);

	//update log level to config
	{
		freertos_util::logging::Global_logger::get()->set_sev_mask_level(config_struct.log_level);
	}

	//timesync processing
	//currently config then no-op
	timesync_task.launch("timesync", 1);

	//led blink/strobe
	led_task.launch("led", 1);

	if(!init_usb())
	{
		logger->log(LOG_LEVEL::ERROR, "main", "USB init failed");
	}
	
	//USB polling
	test_usb_core.launch("usb_core", 1);

	//CPU load & stack info
	system_mon_task.launch("SysMon", 2);

	//start logging_task
	logging_task.launch("logging", 3);
	
	can_usb_app.get_can_tx().set_can_instance(FDCAN1);
	can_usb_app.get_can_tx().set_can_handle(&hfdcan1);

	usb_lawicel_task.set_can_tx(&can_usb_app.get_can_tx());
	usb_lawicel_task.set_usb_rx(&usb_rx_buffer_task);

	//TODO: refactor can handle init
	hfdcan1.Instance = FDCAN1;
	stm32_fdcan_rx_task.set_packet_callback(&can_rx_to_lawicel);
	stm32_fdcan_rx_task.set_can_instance(FDCAN1);
	stm32_fdcan_rx_task.set_can_handle(&hfdcan1);

	//can RX
	stm32_fdcan_rx_task.launch("stm32_fdcan_rx", 4);

	//protocol state machine
	usb_lawicel_task.launch("usb_lawicel", 5);

	//process usb packets
	usb_rx_buffer_task.launch("usb_rx_buf", 6);

	if(config_struct.auto_startup)
	{
		if(!can_usb_app.get_can_tx().open())
		{
			logger->log(LOG_LEVEL::ERROR, "main", "CAN was requested to auto-start, but it failed");
		}
	}

	led_task.set_mode_normal();
	logger->log(LOG_LEVEL::INFO, "main", "Ready");

	for(;;)
	{
		vTaskSuspend(nullptr);
	}
}

bool Main_task::init_usb()
{
	freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();
	using freertos_util::logging::LOG_LEVEL;

	// Startup USB clock and GPIO
	{
		// Hold ext PHY in reset
		HAL_GPIO_WritePin(GPIOA, ULPI_nRESET_Pin, GPIO_PIN_RESET);

		// Start PLL
  		RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
	    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USB;
	    PeriphClkInitStruct.PLL3.PLL3M = 2;
	    PeriphClkInitStruct.PLL3.PLL3N = 16;
	    PeriphClkInitStruct.PLL3.PLL3P = 2;
	    PeriphClkInitStruct.PLL3.PLL3Q = 4;
	    PeriphClkInitStruct.PLL3.PLL3R = 2;
	    PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_3;
	    PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;
	    PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
	    PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_PLL3;
	    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
	    {
	      Error_Handler();
	    }

	    // Init pins
		__HAL_RCC_GPIOC_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();
		/**USB_OTG_HS GPIO Configuration    
		PC0     ------> USB_OTG_HS_ULPI_STP
		PC2_C     ------> USB_OTG_HS_ULPI_DIR
		PC3_C     ------> USB_OTG_HS_ULPI_NXT
		PA3     ------> USB_OTG_HS_ULPI_D0
		PA5     ------> USB_OTG_HS_ULPI_CK
		PB0     ------> USB_OTG_HS_ULPI_D1
		PB1     ------> USB_OTG_HS_ULPI_D2
		PB10     ------> USB_OTG_HS_ULPI_D3
		PB11     ------> USB_OTG_HS_ULPI_D4
		PB12     ------> USB_OTG_HS_ULPI_D5
		PB13     ------> USB_OTG_HS_ULPI_D6
		PB5     ------> USB_OTG_HS_ULPI_D7 
		*/
		GPIO_InitTypeDef GPIO_InitStruct = {0};
		GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_2|GPIO_PIN_3;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF10_OTG2_HS;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		GPIO_InitStruct = {0};
		GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_5;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF10_OTG2_HS;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		GPIO_InitStruct = {0};
		GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_10|GPIO_PIN_11 
		                      |GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_5;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF10_OTG2_HS;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		__HAL_RCC_USB_OTG_HS_CLK_ENABLE();
		__HAL_RCC_USB_OTG_HS_ULPI_CLK_ENABLE();

		// Start external clock and release phy from reset
		{
			HAL_GPIO_WritePin(GPIOA, ULPI_CLK_EN_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOA, ULPI_nRESET_Pin, GPIO_PIN_RESET);

			vTaskDelay(pdMS_TO_TICKS(50));

			HAL_GPIO_WritePin(GPIOA, ULPI_nRESET_Pin, GPIO_PIN_SET);

			vTaskDelay(pdMS_TO_TICKS(50));
		}
		
		USB_OTG_HS->GCCFG &= ~USB_OTG_GCCFG_VBDEN;

		USB_OTG_HS->GOTGCTL |= USB_OTG_GOTGCTL_BVALOEN;
		USB_OTG_HS->GOTGCTL |= USB_OTG_GOTGCTL_BVALOVAL;

		HAL_PWREx_EnableUSBVoltageDetector();
	}

	// Start tinyusb
	{
		NVIC_SetPriority(OTG_HS_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
		// NVIC_EnableIRQ(OTG_HS_IRQn);

		tusb_rhport_init_t dev_init = {
			.role = TUSB_ROLE_DEVICE,
			.speed = TUSB_SPEED_AUTO
		};
		tusb_init(1, &dev_init);

		// tud_cdc_configure_t cdc_init = {
		// 	.rx_persistent = 0;
		// 	.tx_persistent = 0;
		// 	.tx_overwritabe_if_not_connected = 1;
		// };
		// tud_cdc_configure(&cdc_init);
	}

	return true;
}

bool Main_task::mount_fs()
{
	freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();

	logger->log(LOG_LEVEL::INFO, "main", "Ready");

	{
		std::lock_guard<Mutex_static_recursive> lock(can_usb_app.get_mutex());

		W25Q16JV& m_qspi = can_usb_app.get_flash();
		W25Q16JV_conf_region& m_fs = can_usb_app.get_fs();

		m_qspi.set_handle(&hqspi);

		if(!m_qspi.init())
		{
			logger->log(LOG_LEVEL::ERROR, "main", "m_qspi.init failed");

			for(;;)
			{
				vTaskSuspend(nullptr);
			}
		}

		m_fs.initialize();
		m_fs.set_flash(&m_qspi);

		uint8_t mfg_id = 0;
		uint16_t flash_pn = 0;
		if(m_qspi.get_jdec_id(&mfg_id, &flash_pn))
		{
			logger->log(LOG_LEVEL::INFO, "main", "flash mfg id %02" PRIX32, uint32_t(mfg_id));
			logger->log(LOG_LEVEL::INFO, "main", "flash pn %04" PRIX32, uint32_t(flash_pn));
		}
		else
		{
			logger->log(LOG_LEVEL::ERROR, "main", "get_jdec_id failed");
		}

		uint64_t unique_id = 0;
		if(m_qspi.get_unique_id(&unique_id))
		{
			// logger->log(LOG_LEVEL::INFO, "main", "flash sn %016" PRIX64, unique_id);
			//aparently PRIX64 is broken
			logger->log(LOG_LEVEL::INFO, "main", "flash sn %08" PRIX32 "%08" PRIX32, Byte_util::get_upper_half(unique_id), Byte_util::get_lower_half(unique_id));
		}
		else
		{
			logger->log(LOG_LEVEL::ERROR, "main", "get_unique_id failed");
		}

		logger->log(LOG_LEVEL::INFO, "main", "Mounting flash fs");
		int mount_ret = m_fs.mount();
		if(mount_ret != SPIFFS_OK)
		{
			logger->log(LOG_LEVEL::ERROR, "main", "Flash mount failed: %d", mount_ret);
			logger->log(LOG_LEVEL::ERROR, "main", "You will need to reload the config");

			logger->log(LOG_LEVEL::INFO, "main", "Format flash");
			int format_ret = m_fs.format();
			if(format_ret != SPIFFS_OK)
			{
				logger->log(LOG_LEVEL::FATAL, "main", "Flash format failed: %d", format_ret);
				logger->log(LOG_LEVEL::FATAL, "main", "Try a power cycle, your board may be broken");

				for(;;)
				{
					vTaskSuspend(nullptr);
				}
			}

			logger->log(LOG_LEVEL::INFO, "main", "Mounting flash fs");
			mount_ret = m_fs.mount();
			if(mount_ret != SPIFFS_OK)
			{
				logger->log(LOG_LEVEL::FATAL, "main", "Flash mount failed right after we formatted it: %d", mount_ret);
				logger->log(LOG_LEVEL::FATAL, "main", "Try a power cycle, your board may be broken");

				for(;;)
				{
					vTaskSuspend(nullptr);
				}
			}
			else
			{
				logger->log(LOG_LEVEL::INFO, "main", "Flash mount ok");
			}

			//write default config
			if(!can_usb_app.write_default_config())
			{
				logger->log(LOG_LEVEL::FATAL, "main", "Writing default config failed");

				for(;;)
				{
					vTaskSuspend(nullptr);
				}
			}
			if(!can_usb_app.write_default_bitrate_table())
			{
				logger->log(LOG_LEVEL::FATAL, "main", "Writing default bitrate table failed");

				for(;;)
				{
					vTaskSuspend(nullptr);
				}
			}
		}
		else
		{
			logger->log(LOG_LEVEL::INFO, "main", "Flash mount ok");
		}
	}

	return true;
}

bool Main_task::load_config()
{
	freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();

	logger->log(LOG_LEVEL::INFO, "main", "Load config");
	if(!can_usb_app.load_config())
	{
		logger->log(LOG_LEVEL::WARN, "main", "Config load failed, restoring default");

		if(!can_usb_app.write_default_config())
		{
			logger->log(LOG_LEVEL::FATAL, "main", "Writing default config load failed");

			for(;;)
			{
				vTaskSuspend(nullptr);
			}
		}
		else
		{
			logger->log(LOG_LEVEL::WARN, "main", "Default config wrote ok");
		}
	}
	else
	{
		logger->log(LOG_LEVEL::INFO, "main", "Config ok");
	}

	if(!can_usb_app.load_bitrate_table())
	{
		logger->log(LOG_LEVEL::WARN, "main", "Bitrate table load failed, restoring default");

		if(!can_usb_app.write_default_bitrate_table())
		{
			logger->log(LOG_LEVEL::FATAL, "main", "Writing default bitrate table failed");

			for(;;)
			{
				vTaskSuspend(nullptr);
			}
		}
		else
		{
			logger->log(LOG_LEVEL::WARN, "main", "Default bitrate table wrote ok");
		}
	}
	else
	{
		logger->log(LOG_LEVEL::INFO, "main", "Bitrate table ok");
	}

	return true;
}

bool Main_task::handle_usb_set_config_thunk(void* ctx, const uint16_t config)
{
	return static_cast<Main_task*>(ctx)->handle_usb_set_config(config);
}

bool Main_task::handle_usb_set_config(const uint8_t config)
{
	bool ret = false;

	switch(config)
	{
		case 0:
		{
			ret = true;
			break;
		}
		case 1:
		{
			ret = true;
			break;
		}
		default:
		{
			ret = false;
			break;
		}
	}

	return ret;
}
