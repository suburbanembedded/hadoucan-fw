#pragma once

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_fdcan.h"

class stm32_fdcan
{
public:

	void set_can(FDCAN_GlobalTypeDef* can)
	{
		m_fdcan = can;
	}

	bool init();

	bool open();
	bool close();

	bool tx_std(const uint32_t id, const uint8_t dlc, const uint8_t* data);
	bool tx_ext(const uint32_t id, const uint8_t dlc, const uint8_t* data);
	bool tx_rtr_std(const uint32_t id, const uint8_t dlc, const uint8_t* data);
	bool tx_rtr_ext(const uint32_t id, const uint8_t dlc, const uint8_t* data);

	bool tx_fd_std(const uint32_t id, const uint8_t dlc, const uint8_t* data);
	bool tx_fd_ext(const uint32_t id, const uint8_t dlc, const uint8_t* data);
	bool tx_fd_rtr_std(const uint32_t id, const uint8_t dlc, const uint8_t* data);
	bool tx_fd_rtr_ext(const uint32_t id, const uint8_t dlc, const uint8_t* data);

protected:
	FDCAN_GlobalTypeDef* m_fdcan;
	FDCAN_HandleTypeDef m_hfdcan;
};
