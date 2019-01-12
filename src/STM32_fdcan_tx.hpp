#pragma once

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_fdcan.h"

class STM32_fdcan_tx
{
public:

	STM32_fdcan_tx()
	{
		m_is_open = false;
		m_fdcan = nullptr;
	}

	void set_can_instance(FDCAN_GlobalTypeDef* can)
	{
		m_fdcan = can;
	}

	void set_can_handle(FDCAN_HandleTypeDef* const can_handle)
	{
		m_fdcan_handle = can_handle;
	}

	bool init();

	bool open();
	bool close();

	bool tx_std(const uint32_t id, const uint8_t dlc, const uint8_t* data);
	bool tx_ext(const uint32_t id, const uint8_t dlc, const uint8_t* data);
	bool tx_std_rtr(const uint32_t id, const uint8_t dlc);
	bool tx_ext_rtr(const uint32_t id, const uint8_t dlc);

	bool tx_fd_std(const uint32_t id, const uint8_t dlc, const uint8_t* data);
	bool tx_fd_ext(const uint32_t id, const uint8_t dlc, const uint8_t* data);
	bool tx_fd_rtr_std(const uint32_t id, const uint8_t dlc);
	bool tx_fd_rtr_ext(const uint32_t id, const uint8_t dlc);

	enum class STD_BAUD
	{
		B10000,
		B50000,
		B100000,
		B125000,
		B250000,
		B500000,
		B1000000,
	};

	enum class FD_BAUD
	{
		B1000000,
		B2000000,
		B4000000,
		B8000000,
		B12000000
	};

	static bool set_baud(const STD_BAUD baud, FDCAN_HandleTypeDef* const handle);
	static bool set_baud(const STD_BAUD std_baud, const FD_BAUD fd_baud, FDCAN_HandleTypeDef* const handle);

protected:

	bool m_is_open;

	bool send_packet(FDCAN_TxHeaderTypeDef& tx_head, uint8_t* data);

	FDCAN_GlobalTypeDef* m_fdcan;
	FDCAN_HandleTypeDef* m_fdcan_handle;
};

