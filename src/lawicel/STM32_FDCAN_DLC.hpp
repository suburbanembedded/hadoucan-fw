#pragma once

#include "CAN_DLC.hpp"

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_fdcan.h"

#include <cstdint>
#include <cstddef>

class STM32_FDCAN_DLC
{
public:
	size_t to_len() const;
	bool from_len(uint8_t len);

	CAN_DLC to_can_dlc() const;
	bool from_can_dlc(const CAN_DLC& dlc);

	uint32_t get_fdcan_dlc()
	{
		return m_dlc;
	}

protected:

	uint32_t m_dlc;
};