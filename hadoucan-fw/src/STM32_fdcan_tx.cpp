#include "STM32_fdcan_tx.hpp"

#include "lawicel/STM32_FDCAN_DLC.hpp"

#include "global_app_inst.hpp"

#include "main.h"

#include "freertos_cpp_util/logging/Global_logger.hpp"

#include "stm32h7xx_hal_rcc.h"
#include "stm32h7xx_hal_rcc_ex.h"

#include <array>
#include <algorithm>
#include <stdexcept>

bool set_can_clk(const uint32_t can_clk)
{
	freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();
	using freertos_util::logging::LOG_LEVEL;

	logger->log(LOG_LEVEL::INFO, "STM32_fdcan_tx::set_can_clk", "set_can_clk %d", can_clk);

	const uint32_t hse_clk = HSE_VALUE;
	if(hse_clk != 24000000U)
	{
		return false;
	}

	RCC_PeriphCLKInitTypeDef periph_config;
	HAL_RCCEx_GetPeriphCLKConfig(&periph_config);

	bool ret = false;
	switch(periph_config.FdcanClockSelection)
	{
		case RCC_FDCANCLKSOURCE_HSE:
		{
			ret = true;
			break;
		}
		case RCC_FDCANCLKSOURCE_PLL:
		{
			ret = false;
			break;
		}
		case RCC_FDCANCLKSOURCE_PLL2:
		{
			switch(can_clk)
			{
				case 24000000U:
				{
					periph_config.PLL2.PLL2M = 2;
					periph_config.PLL2.PLL2N = 20;
					periph_config.PLL2.PLL2Q = 10;

					ret = true;
					break;
				}
				case 60000000U:
				{
					periph_config.PLL2.PLL2M = 2;
					periph_config.PLL2.PLL2N = 20;
					periph_config.PLL2.PLL2Q = 4;

					ret = true;
					break;
				}
				case 80000000U:
				{
					periph_config.PLL2.PLL2M = 2;
					periph_config.PLL2.PLL2N = 20;
					periph_config.PLL2.PLL2Q = 3;

					ret = true;
					break;
				}
				default:
				{
					ret = false;
					break;
				}
			}

			break;
		}
		default:
		{
			ret = false;
			break;
		}
	}

	if(ret == true)
	{
		if(HAL_RCCEx_PeriphCLKConfig(&periph_config) != HAL_OK)
		{
			ret = false;
		}
	}

	return ret;
}

bool get_can_clk(uint32_t* const can_clk)
{
	const uint32_t hse_clk = HSE_VALUE;

	RCC_PeriphCLKInitTypeDef periph_config;
	HAL_RCCEx_GetPeriphCLKConfig(&periph_config);

	bool ret = false;
	switch(periph_config.FdcanClockSelection)
	{
		case RCC_FDCANCLKSOURCE_HSE:
		{
			*can_clk = hse_clk;
			ret = true;
			break;
		}
		case RCC_FDCANCLKSOURCE_PLL:
		{
			RCC_OscInitTypeDef pll_config;
			HAL_RCC_GetOscConfig(&pll_config);

			const uint32_t m = pll_config.PLL.PLLM;
			const uint32_t n = pll_config.PLL.PLLN;
			const uint32_t q = pll_config.PLL.PLLQ;

			*can_clk = ((hse_clk / m) * n) / q;
			ret = true;
			break;
		}
		case RCC_FDCANCLKSOURCE_PLL2:
		{
			const uint32_t m = periph_config.PLL2.PLL2M;
			const uint32_t n = periph_config.PLL2.PLL2N;
			const uint32_t q = periph_config.PLL2.PLL2Q;

			*can_clk = ((hse_clk / m) * n) / q;
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

bool STM32_fdcan_tx::init()
{
	freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();
	using freertos_util::logging::LOG_LEVEL;

	logger->log(LOG_LEVEL::DEBUG, "STM32_fdcan_tx::init", "");

	HAL_StatusTypeDef ret = HAL_OK;

	//TODO: store the baud rate and always reconfigure it here
	*m_fdcan_handle = FDCAN_HandleTypeDef();

	m_fdcan_handle->Instance = m_fdcan;

	{
		std::unique_lock<Mutex_static_recursive> config_lock;
		const CAN_USB_app_config::Config_Set& m_config = can_usb_app.get_config(&config_lock);

		//classic, no brs, brs
		logger->log(LOG_LEVEL::TRACE, "STM32_fdcan_tx::init", "protocol");
		if(m_config.protocol_fd)
		{
			if(m_config.protocol_brs)
			{
				m_fdcan_handle->Init.FrameFormat = FDCAN_FRAME_FD_BRS;
			}
			else
			{
				m_fdcan_handle->Init.FrameFormat = FDCAN_FRAME_FD_NO_BRS;
			}
		}
		else
		{
			m_fdcan_handle->Init.FrameFormat = FDCAN_FRAME_CLASSIC;
		}
		
		if(m_config.listen_only)
		{
			m_fdcan_handle->Init.Mode = FDCAN_MODE_BUS_MONITORING;
		}
		else
		{
			m_fdcan_handle->Init.Mode = FDCAN_MODE_NORMAL;
		}

		m_fdcan_handle->Init.AutoRetransmission = ENABLE;
		m_fdcan_handle->Init.TransmitPause = DISABLE;
		m_fdcan_handle->Init.ProtocolException = ENABLE;

		//handle the slew rate control based on the setting and baud rate
		logger->log(LOG_LEVEL::TRACE, "STM32_fdcan_tx::init", "slope_ctrl");
		switch(m_config.slope_ctrl)
		{
			case CAN_USB_app_config::SLOPE_CONTROL::SLOW:
			{
				set_can_slew_slow();
				break;
			}
			case CAN_USB_app_config::SLOPE_CONTROL::FAST:
			{
				set_can_slew_high();
				break;
			}
			default:
			{
				logger->log(LOG_LEVEL::WARN, "STM32_fdcan_tx::init", "slope control setting corrupt, setting to auto");
				//fall through here
			}
			case CAN_USB_app_config::SLOPE_CONTROL::AUTO:
			{
				if(m_config.protocol_brs)
				{
					if((m_config.bitrate_nominal > 500000) || (m_config.bitrate_data > 500000))
					{
						set_can_slew_high();
					}
					else
					{
						set_can_slew_slow();
					}
				}
				else
				{
					if(m_config.bitrate_nominal > 500000)
					{
						set_can_slew_high();
					}
					else
					{
						set_can_slew_slow();
					}
				}
				break;
			}
		}

		logger->log(LOG_LEVEL::TRACE, "STM32_fdcan_tx::init", "set baud");
		if(!set_baud(m_config.bitrate_nominal, m_config.bitrate_data))
		{
			logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_tx::init", "set_baud failed");
			return false;
		}
	}

	m_fdcan_handle->Init.MessageRAMOffset = 0;
	m_fdcan_handle->Init.StdFiltersNbr = 1;
	m_fdcan_handle->Init.ExtFiltersNbr = 1;
	m_fdcan_handle->Init.RxFifo0ElmtsNbr = 64;
	m_fdcan_handle->Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_64;
	m_fdcan_handle->Init.RxFifo1ElmtsNbr = 0;
	m_fdcan_handle->Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_64;
	m_fdcan_handle->Init.RxBuffersNbr = 0;

	m_fdcan_handle->Init.TxEventsNbr = 0;
	m_fdcan_handle->Init.TxBuffersNbr = 0;
	m_fdcan_handle->Init.TxFifoQueueElmtsNbr = 32;
	m_fdcan_handle->Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
	m_fdcan_handle->Init.TxElmtSize = FDCAN_DATA_BYTES_64;

	logger->log(LOG_LEVEL::TRACE, "STM32_fdcan_tx::init", "HAL_FDCAN_Init");
	ret = HAL_FDCAN_Init(m_fdcan_handle);
	if(ret != HAL_OK)
	{
		logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_tx::init", "HAL_FDCAN_Init failed");
		return false;
	}

	//bypass clock calibration
	// fdcan_ker_ck = 60MHz
	// fdcan_tq_ck  = fdcan_ker_ck / 1
	logger->log(LOG_LEVEL::TRACE, "STM32_fdcan_tx::init", "HAL_FDCAN_ConfigClockCalibration");
	FDCAN_ClkCalUnitTypeDef cal_config = FDCAN_ClkCalUnitTypeDef();
	cal_config.ClockCalibration = DISABLE;
	cal_config.ClockDivider = FDCAN_CLOCK_DIV1;
	ret = HAL_FDCAN_ConfigClockCalibration(m_fdcan_handle, &cal_config);
	if(ret != HAL_OK)
	{
		logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_tx::init", "HAL_FDCAN_ConfigClockCalibration failed");
		return false;
	}

	//units of mtq - fdcan_tq_ck - 60MHz -> 16 2/3 ns
	//ADM3055E - TXD->BUS R->D 35 - 60ns
	//ADM3055E - TXD->BUS D->E 46 - 70ns
	//ADM3055E - TXD->RXD Falling 150ns full, 300ns slope ctrl
	//ADM3055E - TXD->RXD Rising 150ns full, 300ns slope ctrl
	//150ns is 9 mtq
	logger->log(LOG_LEVEL::TRACE, "STM32_fdcan_tx::init", "HAL_FDCAN_ConfigTxDelayCompensation");
	ret = HAL_FDCAN_ConfigTxDelayCompensation(m_fdcan_handle, 5, 0);
	if(ret != HAL_OK)
	{
		logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_tx::init", "HAL_FDCAN_ConfigTxDelayCompensation failed");
		return false;
	}
	logger->log(LOG_LEVEL::TRACE, "STM32_fdcan_tx::init", "HAL_FDCAN_EnableTxDelayCompensation");
	ret = HAL_FDCAN_EnableTxDelayCompensation(m_fdcan_handle);
	if(ret != HAL_OK)
	{
		logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_tx::init", "HAL_FDCAN_EnableTxDelayCompensation failed");
		return false;
	}

	logger->log(LOG_LEVEL::TRACE, "STM32_fdcan_tx::init", "HAL_FDCAN_GetErrorCounters");
	FDCAN_ErrorCountersTypeDef error_counters;
	ret = HAL_FDCAN_GetErrorCounters(m_fdcan_handle, &error_counters);
	if(ret != HAL_OK)
	{
		logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_tx::init", "HAL_FDCAN_GetErrorCounters failed");
		return false;
	}

	logger->log(LOG_LEVEL::TRACE, "STM32_fdcan_tx::init", "HAL_FDCAN_ConfigTimestampCounter");
	ret = HAL_FDCAN_ConfigTimestampCounter(m_fdcan_handle, FDCAN_TIMESTAMP_PRESC_1);
	if(ret != HAL_OK)
	{
		logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_tx::init", "HAL_FDCAN_ConfigTimestampCounter failed");
		return false;
	}

	logger->log(LOG_LEVEL::TRACE, "STM32_fdcan_tx::init", "HAL_FDCAN_EnableTimestampCounter");
	HAL_FDCAN_EnableTimestampCounter(m_fdcan_handle, FDCAN_TIMESTAMP_EXTERNAL);
	if(ret != HAL_OK)
	{
		logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_tx::init", "HAL_FDCAN_EnableTimestampCounter failed");
		return false;
	}

	// Configure Rx Std filter
	logger->log(LOG_LEVEL::TRACE, "STM32_fdcan_tx::init", "HAL_FDCAN_ConfigFilter RX STD");
	FDCAN_FilterTypeDef sFilter0;
	sFilter0.IdType = FDCAN_STANDARD_ID;
	sFilter0.FilterIndex = 0;
	sFilter0.FilterType = FDCAN_FILTER_MASK;
	sFilter0.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
	sFilter0.FilterID1 = 0x000;//filter
	// sFilter0.FilterID2 = 0x7FF;//mask all
	sFilter0.FilterID2 = 0x000;//mask none, match all
	ret = HAL_FDCAN_ConfigFilter(m_fdcan_handle, &sFilter0);
	if(ret != HAL_OK)
	{
		logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_tx::init", "HAL_FDCAN_ConfigFilter failed");
		return false;
	}

	// Configure Rx Ext filter
	logger->log(LOG_LEVEL::TRACE, "STM32_fdcan_tx::init", "HAL_FDCAN_ConfigFilter RX EXT");
	FDCAN_FilterTypeDef sFilter1;
	sFilter1.IdType = FDCAN_EXTENDED_ID;
	sFilter1.FilterIndex = 0;
	sFilter1.FilterType = FDCAN_FILTER_MASK;
	sFilter1.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
	sFilter1.FilterID1 = 0x00000000;//filter
	// sFilter1.FilterID2 = 0x1FFFFFFF;//mask
	sFilter1.FilterID2 = 0x00000000;//mask none, match all
	ret = HAL_FDCAN_ConfigFilter(m_fdcan_handle, &sFilter1);
	if(ret != HAL_OK)
	{
		logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_tx::init", "HAL_FDCAN_ConfigFilter failed");
		return false;
	}

	logger->log(LOG_LEVEL::TRACE, "STM32_fdcan_tx::init", "HAL_FDCAN_ConfigFifoWatermark");
	ret = HAL_FDCAN_ConfigFifoWatermark(m_fdcan_handle, FDCAN_CFG_RX_FIFO0, 16);
	if(ret != HAL_OK)
	{
		logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_tx::init", "HAL_FDCAN_ConfigFifoWatermark for FIFO0 failed");
		return false;
	}

	logger->log(LOG_LEVEL::TRACE, "STM32_fdcan_tx::init", "HAL_FDCAN_ConfigRxFifoOverwrite");
	ret = HAL_FDCAN_ConfigRxFifoOverwrite(m_fdcan_handle, FDCAN_CFG_RX_FIFO0, FDCAN_RX_FIFO_OVERWRITE);
	if(ret != HAL_OK)
	{
		logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_tx::init", "HAL_FDCAN_ConfigRxFifoOverwrite for FIFO0 failed");
		return false;
	}

	logger->log(LOG_LEVEL::TRACE, "STM32_fdcan_tx::init", "HAL_FDCAN_ActivateNotification");
	ret = HAL_FDCAN_ActivateNotification(m_fdcan_handle, FDCAN_IT_RX_FIFO0_WATERMARK | FDCAN_IT_RX_FIFO0_FULL | FDCAN_IT_RX_FIFO0_MESSAGE_LOST, 0);
	if(ret != HAL_OK)
	{
		logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_tx::init", "HAL_FDCAN_ActivateNotification for FIFO0 failed");
		return false;
	}

	// ret = HAL_FDCAN_ConfigFifoWatermark(m_fdcan_handle, FDCAN_CFG_RX_FIFO1, 2);
	// if(ret != HAL_OK)
	// {
	// 	logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_tx::init", "HAL_FDCAN_ConfigFifoWatermark for FIFO1 failed");
	// 	return false;
	// }

	// ret = HAL_FDCAN_ConfigRxFifoOverwrite(m_fdcan_handle, FDCAN_CFG_RX_FIFO1, FDCAN_RX_FIFO_OVERWRITE);
	// if(ret != HAL_OK)
	// {
	// 	logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_tx::init", "HAL_FDCAN_ConfigRxFifoOverwrite for FIFO1 failed");
	// 	return false;
	// }

	// ret = HAL_FDCAN_ActivateNotification(m_fdcan_handle, FDCAN_IT_RX_FIFO1_WATERMARK | FDCAN_IT_RX_FIFO1_FULL | FDCAN_IT_RX_FIFO1_MESSAGE_LOST, 0);
	// ret = HAL_FDCAN_ActivateNotification(m_fdcan_handle, FDCAN_IT_RX_FIFO1_WATERMARK, 0);
	// if(ret != HAL_OK)
	// {
	// 	logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_tx::init", "HAL_FDCAN_ActivateNotification for FIFO1 failed");
	// 	return false;
	// }

	// ret = HAL_FDCAN_ConfigGlobalFilter(m_fdcan_handle, FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_ACCEPT_IN_RX_FIFO1, DISABLE, DISABLE);
	logger->log(LOG_LEVEL::TRACE, "STM32_fdcan_tx::init", "HAL_FDCAN_ConfigGlobalFilter");
	ret = HAL_FDCAN_ConfigGlobalFilter(m_fdcan_handle, FDCAN_REJECT, FDCAN_REJECT, DISABLE, DISABLE);
	if(ret != HAL_OK)
	{
		logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_tx::init", "HAL_FDCAN_ConfigGlobalFilter failed");
		return false;
	}

	{
		bool fd_iso_mode = true;
		{
			std::unique_lock<Mutex_static_recursive> config_lock;
			const CAN_USB_app_config::Config_Set& m_config = can_usb_app.get_config(&config_lock);

			fd_iso_mode = m_config.protocol_fd_iso;
		}

		if(fd_iso_mode)
		{
			logger->log(LOG_LEVEL::DEBUG, "STM32_fdcan_tx::init", "ISO FD mode requested");
			ret = HAL_FDCAN_EnableISOMode(m_fdcan_handle);
			if(ret != HAL_OK)
			{
				logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_tx::init", "HAL_FDCAN_EnableISOMode failed");
				return false;
			}
		}
		else
		{
			logger->log(LOG_LEVEL::DEBUG, "STM32_fdcan_tx::init", "Non-ISO FD mode requested");
			ret = HAL_FDCAN_DisableISOMode(m_fdcan_handle);
			if(ret != HAL_OK)
			{
				logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_tx::init", "HAL_FDCAN_DisableISOMode failed");
				return false;
			}
		}
	}

	return true;
}

bool STM32_fdcan_tx::set_baud(const int std_baud)
{
	freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();
	using freertos_util::logging::LOG_LEVEL;

	CAN_USB_app_bitrate_table::Bitrate_Table_Entry nominal_entry;

	{
		std::unique_lock<Mutex_static_recursive> config_lock;
		const CAN_USB_app_config::Config_Set& m_config = can_usb_app.get_config(&config_lock);
		
		std::unique_lock<Mutex_static_recursive> bitrate_table_lock;
		const CAN_USB_app_bitrate_table& m_bitrate_table = can_usb_app.get_bitrate_tables(&bitrate_table_lock);

		if(!m_bitrate_table.get_nominal_entry(m_config.can_clock, std_baud, &nominal_entry))
		{
			logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_tx::set_baud", "m_bitrate_table.get_nominal_entry failed, clock: %d, baud: %d", m_config.can_clock, std_baud);
			return false;
		}
	}

	return set_baud(nominal_entry, m_fdcan_handle);
}

bool STM32_fdcan_tx::set_baud(const CAN_USB_app_bitrate_table::Bitrate_Table_Entry& std_baud, FDCAN_HandleTypeDef* const handle)
{
	handle->Init.NominalPrescaler = std_baud.pre;      //1-512
	handle->Init.NominalSyncJumpWidth = std_baud.sjw;  //1-128
	handle->Init.NominalTimeSeg1 = std_baud.tseg1;     //1-256 
	handle->Init.NominalTimeSeg2 = std_baud.tseg2;     //1-128

	return true;
}

bool STM32_fdcan_tx::set_baud(const int std_baud, const int fd_baud)
{
	freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();
	using freertos_util::logging::LOG_LEVEL;

	CAN_USB_app_bitrate_table::Bitrate_Table_Entry nominal_entry;
	CAN_USB_app_bitrate_table::Bitrate_Table_Entry data_entry;

	{
		std::unique_lock<Mutex_static_recursive> config_lock;
		const CAN_USB_app_config::Config_Set& m_config = can_usb_app.get_config(&config_lock);
		
		std::unique_lock<Mutex_static_recursive> bitrate_table_lock;
		const CAN_USB_app_bitrate_table& m_bitrate_table = can_usb_app.get_bitrate_tables(&bitrate_table_lock);

		if(!m_bitrate_table.get_nominal_entry(m_config.can_clock, std_baud, &nominal_entry))
		{
			logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_tx::set_baud", "m_bitrate_table.get_nominal_entry failed, clock: %d, baud: %d", m_config.can_clock, std_baud);
			return false;	
		}

		if(!m_bitrate_table.get_data_entry(m_config.can_clock, fd_baud, &data_entry))
		{
			logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_tx::set_baud", "m_bitrate_table.get_data_entry failed, clock: %d, baud: %d", m_config.can_clock, fd_baud);
			return false;
		}
	}

	return set_baud(nominal_entry, data_entry, m_fdcan_handle);
}
bool STM32_fdcan_tx::set_baud(const CAN_USB_app_bitrate_table::Bitrate_Table_Entry& std_baud, const CAN_USB_app_bitrate_table::Bitrate_Table_Entry& fd_baud, FDCAN_HandleTypeDef* const handle)
{

	handle->Init.NominalPrescaler = std_baud.pre;      //1-512
	handle->Init.NominalSyncJumpWidth = std_baud.sjw;  //1-128
	handle->Init.NominalTimeSeg1 = std_baud.tseg1;     //1-256 
	handle->Init.NominalTimeSeg2 = std_baud.tseg2;     //1-128

	handle->Init.DataPrescaler = fd_baud.pre;      //1-32
	handle->Init.DataSyncJumpWidth = fd_baud.sjw;  //1-16
	handle->Init.DataTimeSeg1 = fd_baud.tseg1;     //1-32 
	handle->Init.DataTimeSeg2 = fd_baud.tseg2;     //1-16

	return true;
}

bool STM32_fdcan_tx::open()
{
	freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();
	using freertos_util::logging::LOG_LEVEL;

	logger->log(LOG_LEVEL::DEBUG, "STM32_fdcan_tx::open", "");

	HAL_StatusTypeDef ret = HAL_OK;

	logger->log(LOG_LEVEL::TRACE, "STM32_fdcan_tx::open", "init");
	if(!init())
	{
		logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_tx::open", "STM32_fdcan_tx::init failed");
		return false;
	}

	logger->log(LOG_LEVEL::TRACE, "STM32_fdcan_tx::open", "start");
	ret = HAL_FDCAN_Start(m_fdcan_handle);
	if(ret != HAL_OK)
	{
		logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_tx::open", "HAL_FDCAN_Start failed");
		return false;
	}

	logger->log(LOG_LEVEL::TRACE, "STM32_fdcan_tx::open", "mode");
	if(HAL_FDCAN_IsRestrictedOperationMode(m_fdcan_handle))
	{
		logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_tx::open", "FDCAN is in Restricted Mode");
		return false;	
	}

	logger->log(LOG_LEVEL::INFO, "STM32_fdcan_tx::open", "CAN is open");
	m_is_open = true;

	return true;
}
bool STM32_fdcan_tx::close()
{
	freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();
	using freertos_util::logging::LOG_LEVEL;

	logger->log(LOG_LEVEL::DEBUG, "STM32_fdcan_tx::close", "");

	// if(HAL_FDCAN_DeactivateNotification(m_fdcan_handle, FDCAN_IT_RX_FIFO0_WATERMARK | FDCAN_IT_RX_FIFO0_FULL | FDCAN_IT_RX_FIFO0_MESSAGE_LOST) != HAL_OK)
	if(HAL_FDCAN_DeactivateNotification(m_fdcan_handle, FDCAN_IT_RX_FIFO0_WATERMARK) != HAL_OK)
	{
		logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_tx::close", "HAL_FDCAN_DeactivateNotification for FIFO0 failed");
		return false;	
	}
/*
	// if(HAL_FDCAN_DeactivateNotification(m_fdcan_handle, FDCAN_IT_RX_FIFO1_WATERMARK | FDCAN_IT_RX_FIFO1_FULL | FDCAN_IT_RX_FIFO1_MESSAGE_LOST) != HAL_OK)
	if(HAL_FDCAN_DeactivateNotification(m_fdcan_handle, FDCAN_IT_RX_FIFO1_WATERMARK) != HAL_OK)
	{
		logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_tx::close", "HAL_FDCAN_DeactivateNotification for FIFO1 failed");
		return false;	
	}
*/
	if(HAL_FDCAN_Stop(m_fdcan_handle) != HAL_OK)
	{
		return false;
	}

	logger->log(LOG_LEVEL::INFO, "STM32_fdcan_tx::open", "CAN is closed");
	m_is_open = false;

	return true;
}

bool STM32_fdcan_tx::tx_std(const uint32_t id, const uint8_t data_len, const uint8_t* data)
{
	freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();
	using freertos_util::logging::LOG_LEVEL;

	logger->log(LOG_LEVEL::TRACE, "STM32_fdcan_tx::tx_std", "");

	if(!m_is_open)
	{
		logger->log(LOG_LEVEL::WARN, "STM32_fdcan_tx::tx_std", "Tried to send with closed interface");
		return false;
	}

	STM32_FDCAN_DLC dlc;
	if(!dlc.from_len(data_len))
	{
		return false;
	}

	FDCAN_TxHeaderTypeDef tx_head;

	tx_head.Identifier = id;
	tx_head.IdType = FDCAN_STANDARD_ID;
	tx_head.TxFrameType = FDCAN_DATA_FRAME;
	tx_head.DataLength = dlc.get_fdcan_dlc();
	tx_head.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	tx_head.BitRateSwitch = FDCAN_BRS_OFF;
	tx_head.FDFormat = FDCAN_CLASSIC_CAN;
	tx_head.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	tx_head.MessageMarker = 0;

	std::array<uint8_t, 8> out_data;
	std::copy_n(data, data_len, out_data.begin());

	return send_packet(tx_head, out_data.data());
}

bool STM32_fdcan_tx::tx_ext(const uint32_t id, const uint8_t data_len, const uint8_t* data)
{
	freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();
	using freertos_util::logging::LOG_LEVEL;

	logger->log(LOG_LEVEL::TRACE, "STM32_fdcan_tx::tx_ext", "");

	if(!m_is_open)
	{
		logger->log(LOG_LEVEL::WARN, "STM32_fdcan_tx::tx_ext", "Tried to send with closed interface");
		return false;
	}

	STM32_FDCAN_DLC dlc;
	if(!dlc.from_len(data_len))
	{
		return false;
	}

	FDCAN_TxHeaderTypeDef tx_head;

	tx_head.Identifier = id;
	tx_head.IdType = FDCAN_EXTENDED_ID;
	tx_head.TxFrameType = FDCAN_DATA_FRAME;
	tx_head.DataLength = dlc.get_fdcan_dlc();
	tx_head.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	tx_head.BitRateSwitch = FDCAN_BRS_OFF;
	tx_head.FDFormat = FDCAN_CLASSIC_CAN;
	tx_head.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	tx_head.MessageMarker = 0;

	std::array<uint8_t, 8> out_data;
	std::copy_n(data, data_len, out_data.begin());

	return send_packet(tx_head, out_data.data());
}

bool STM32_fdcan_tx::tx_std_rtr(const uint32_t id, const uint8_t data_len)
{
	freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();
	using freertos_util::logging::LOG_LEVEL;

	logger->log(LOG_LEVEL::TRACE, "STM32_fdcan_tx::tx_std_rtr", "");

	if(!m_is_open)
	{
		logger->log(LOG_LEVEL::WARN, "STM32_fdcan_tx::tx_std_rtr", "Tried to send with closed interface");
		return false;
	}

	STM32_FDCAN_DLC dlc;
	if(!dlc.from_len(data_len))
	{
		return false;
	}

	FDCAN_TxHeaderTypeDef tx_head;

	tx_head.Identifier = id;
	tx_head.IdType = FDCAN_STANDARD_ID;
	tx_head.TxFrameType = FDCAN_REMOTE_FRAME;
	tx_head.DataLength = dlc.get_fdcan_dlc();
	tx_head.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	tx_head.BitRateSwitch = FDCAN_BRS_OFF;
	tx_head.FDFormat = FDCAN_CLASSIC_CAN;
	tx_head.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	tx_head.MessageMarker = 0;

	return send_packet(tx_head, nullptr);
}
bool STM32_fdcan_tx::tx_ext_rtr(const uint32_t id, const uint8_t data_len)
{
	freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();
	using freertos_util::logging::LOG_LEVEL;

	logger->log(LOG_LEVEL::TRACE, "STM32_fdcan_tx::tx_ext_rtr", "");

	if(!m_is_open)
	{
		logger->log(LOG_LEVEL::WARN, "STM32_fdcan_tx::tx_ext_rtr", "Tried to send with closed interface");
		return false;
	}

	STM32_FDCAN_DLC dlc;
	if(!dlc.from_len(data_len))
	{
		return false;
	}

	FDCAN_TxHeaderTypeDef tx_head;

	tx_head.Identifier = id;
	tx_head.IdType = FDCAN_EXTENDED_ID;
	tx_head.TxFrameType = FDCAN_REMOTE_FRAME;
	tx_head.DataLength = dlc.get_fdcan_dlc();
	tx_head.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	tx_head.BitRateSwitch = FDCAN_BRS_OFF;
	tx_head.FDFormat = FDCAN_CLASSIC_CAN;
	tx_head.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	tx_head.MessageMarker = 0;

	return send_packet(tx_head, nullptr);
}

bool STM32_fdcan_tx::tx_fd_std(const uint32_t id, const BRS brs, const ESI esi, const uint8_t data_len, const uint8_t* data)
{
	freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();
	using freertos_util::logging::LOG_LEVEL;

	logger->log(LOG_LEVEL::TRACE, "STM32_fdcan_tx::tx_fd_std", "");

	if(!m_is_open)
	{
		logger->log(LOG_LEVEL::WARN, "STM32_fdcan_tx::tx_fd_std", "Tried to send with closed interface");
		return false;
	}

	STM32_FDCAN_DLC dlc;
	if(!dlc.from_len(data_len))
	{
		logger->log(LOG_LEVEL::WARN, "STM32_fdcan_tx::tx_fd_std", "STM32_FDCAN_DLC::from_len failed");
		return false;
	}

	FDCAN_TxHeaderTypeDef tx_head;

	tx_head.Identifier = id;
	tx_head.IdType = FDCAN_STANDARD_ID;
	tx_head.TxFrameType = FDCAN_DATA_FRAME;
	tx_head.DataLength = dlc.get_fdcan_dlc();

	switch(esi)
	{
		case ESI::PASSIVE:
		{
			tx_head.ErrorStateIndicator = FDCAN_ESI_PASSIVE;
			break;
		}
		case ESI::ACTIVE:
		{
			tx_head.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
			break;
		}
		default:
		{
			logger->log(LOG_LEVEL::WARN, "STM32_fdcan_tx::tx_fd_std", "ESI conv failed");
			return false;
		}
	}

	switch(brs)
	{
		case BRS::OFF:
		{
			tx_head.BitRateSwitch = FDCAN_BRS_OFF;
			break;
		}
		case BRS::ON:
		{
			tx_head.BitRateSwitch = FDCAN_BRS_ON;
			break;
		}
		default:
		{
			logger->log(LOG_LEVEL::WARN, "STM32_fdcan_tx::tx_fd_std", "BRS conv failed");
			return false;
		}
	}

	tx_head.FDFormat = FDCAN_FD_CAN;
	tx_head.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	tx_head.MessageMarker = 0;

	std::array<uint8_t, 64> out_data;
	std::copy_n(data, data_len, out_data.begin());

	return send_packet(tx_head, out_data.data());
}
bool STM32_fdcan_tx::tx_fd_ext(const uint32_t id, const BRS brs, const ESI esi, const uint8_t data_len, const uint8_t* data)
{
	freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();
	using freertos_util::logging::LOG_LEVEL;

	logger->log(LOG_LEVEL::TRACE, "STM32_fdcan_tx::tx_fd_ext", "");

	if(!m_is_open)
	{
		logger->log(LOG_LEVEL::WARN, "STM32_fdcan_tx::tx_fd_ext", "Tried to send with closed interface");
		return false;
	}

	STM32_FDCAN_DLC dlc;
	if(!dlc.from_len(data_len))
	{
		logger->log(LOG_LEVEL::WARN, "STM32_fdcan_tx::tx_fd_ext", "STM32_FDCAN_DLC::from_len failed");
		return false;
	}

	FDCAN_TxHeaderTypeDef tx_head;
	
	tx_head.Identifier = id;
	tx_head.IdType = FDCAN_EXTENDED_ID;
	tx_head.TxFrameType = FDCAN_DATA_FRAME;
	tx_head.DataLength = dlc.get_fdcan_dlc();

	switch(esi)
	{
		case ESI::PASSIVE:
		{
			tx_head.ErrorStateIndicator = FDCAN_ESI_PASSIVE;
			break;
		}
		case ESI::ACTIVE:
		{
			tx_head.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
			break;
		}
		default:
		{
			logger->log(LOG_LEVEL::WARN, "STM32_fdcan_tx::tx_fd_ext", "ESI conv failed");
			return false;
		}
	}
	
	switch(brs)
	{
		case BRS::OFF:
		{
			tx_head.BitRateSwitch = FDCAN_BRS_OFF;
			break;
		}
		case BRS::ON:
		{
			tx_head.BitRateSwitch = FDCAN_BRS_ON;
			break;
		}
		default:
		{
			logger->log(LOG_LEVEL::WARN, "STM32_fdcan_tx::tx_fd_ext", "BRS conv failed");
			return false;
		}
	}

	tx_head.FDFormat = FDCAN_FD_CAN;
	tx_head.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	tx_head.MessageMarker = 0;

	std::array<uint8_t, 64> out_data;
	std::copy_n(data, data_len, out_data.begin());

	return send_packet(tx_head, out_data.data());
}

bool STM32_fdcan_tx::send_packet(FDCAN_TxHeaderTypeDef& tx_head, uint8_t* data)
{
	freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();
	using freertos_util::logging::LOG_LEVEL;

	logger->log(LOG_LEVEL::TRACE, "STM32_fdcan_tx::send_packet", "");

	size_t retry_counter = 0;
	HAL_StatusTypeDef ret = HAL_OK;
	do
	{
		ret = HAL_FDCAN_AddMessageToTxFifoQ(m_fdcan_handle, &tx_head, data);

		if(ret != HAL_OK)
		{
			logger->log(LOG_LEVEL::DEBUG, "STM32_fdcan_tx::send_packet", "HAL_FDCAN_AddMessageToTxFifoQ failed, overflow?");

			if(retry_counter > 2)
			{
				logger->log(LOG_LEVEL::ERROR, "STM32_fdcan_tx::send_packet", "HAL_FDCAN_AddMessageToTxFifoQ failed, timeout");
				return false;
			}

			retry_counter++;
			vTaskDelay(1);
		}
	} while(ret != HAL_OK);

	return true;
}

void STM32_fdcan_tx::set_can_slew_slow()
{
	freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();
	using freertos_util::logging::LOG_LEVEL;
	
	logger->log(LOG_LEVEL::DEBUG, "STM32_fdcan_tx::set_can_slew_slow", "");

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = CAN_SLOPE_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(CAN_SLOPE_GPIO_Port, &GPIO_InitStruct);

	HAL_GPIO_WritePin(CAN_SLOPE_GPIO_Port, CAN_SLOPE_Pin, GPIO_PIN_RESET);
}
void STM32_fdcan_tx::set_can_slew_high()
{
	freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();
	using freertos_util::logging::LOG_LEVEL;

	logger->log(LOG_LEVEL::DEBUG, "STM32_fdcan_tx::set_can_slew_high", "");

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = CAN_SLOPE_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(CAN_SLOPE_GPIO_Port, &GPIO_InitStruct);

	HAL_GPIO_WritePin(CAN_SLOPE_GPIO_Port, CAN_SLOPE_Pin, GPIO_PIN_SET);
}

extern "C"
{
	// void HAL_FDCAN_ClockCalibrationCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t ClkCalibrationITs);
	// void HAL_FDCAN_TxEventFifoCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t TxEventFifoITs);
	// void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs);
	// void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs);
	// void HAL_FDCAN_TxFifoEmptyCallback(FDCAN_HandleTypeDef *hfdcan);
	// void HAL_FDCAN_TxBufferCompleteCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t BufferIndexes);
	// void HAL_FDCAN_TxBufferAbortCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t BufferIndexes);
	// void HAL_FDCAN_RxBufferNewMessageCallback(FDCAN_HandleTypeDef *hfdcan);
	// void HAL_FDCAN_HighPriorityMessageCallback(FDCAN_HandleTypeDef *hfdcan);
	// void HAL_FDCAN_TimestampWraparoundCallback(FDCAN_HandleTypeDef *hfdcan);
	// void HAL_FDCAN_TimeoutOccurredCallback(FDCAN_HandleTypeDef *hfdcan);
	// void HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef *hfdcan);
}
