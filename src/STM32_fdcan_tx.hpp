#pragma once

#include "CAN_USB_app_config.hpp"
#include "CAN_USB_app_bitrate_table.hpp"

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_fdcan.h"

class STM32_fdcan_tx
{
public:

	enum class BRS
	{
		OFF,
		ON
	};

	enum class ESI
	{
		ACTIVE,//DOM, def
		PASSIVE//recessive, means error
	};

	STM32_fdcan_tx()
	{		
		m_is_open = false;
		m_fdcan = nullptr;

		m_config = CAN_USB_app_config::get_defualt();
	}

	void set_can_instance(FDCAN_GlobalTypeDef* can)
	{
		m_fdcan = can;
	}

	void set_can_handle(FDCAN_HandleTypeDef* const can_handle)
	{
		m_fdcan_handle = can_handle;
	}

	void set_config(const CAN_USB_app_config::Config_Set& config)
	{
		m_config = config;
	}

	void set_bitrate_table(const CAN_USB_app_bitrate_table& table)
	{
		m_bitrate_table = table;
	}

	bool init();

	bool open();
	bool close();

	bool tx_std(const uint32_t id, const uint8_t data_len, const uint8_t* data);
	bool tx_ext(const uint32_t id, const uint8_t data_len, const uint8_t* data);
	bool tx_std_rtr(const uint32_t id, const uint8_t data_len);
	bool tx_ext_rtr(const uint32_t id, const uint8_t data_len);

	bool tx_fd_std(const uint32_t id, const BRS brs, const ESI esi, const uint8_t data_len, const uint8_t* data);
	bool tx_fd_ext(const uint32_t id, const BRS brs, const ESI esi, const uint8_t data_len, const uint8_t* data);

	bool set_baud(const int std_baud);
	bool set_baud(const int std_baud, const int fd_baud);

protected:

	static void set_can_slew_slow();
	static void set_can_slew_high();

	static bool set_baud(const CAN_USB_app_bitrate_table::Bitrate_Table_Entry& std_baud, FDCAN_HandleTypeDef* const handle);
	static bool set_baud(const CAN_USB_app_bitrate_table::Bitrate_Table_Entry& std_baud, const CAN_USB_app_bitrate_table::Bitrate_Table_Entry& fd_baud, FDCAN_HandleTypeDef* const handle);

	CAN_USB_app_config::Config_Set m_config;
	CAN_USB_app_bitrate_table m_bitrate_table;

	bool m_is_open;

	bool send_packet(FDCAN_TxHeaderTypeDef& tx_head, uint8_t* data);

	FDCAN_GlobalTypeDef* m_fdcan;
	FDCAN_HandleTypeDef* m_fdcan_handle;
};

