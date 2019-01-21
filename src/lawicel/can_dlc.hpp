#pragma once

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_fdcan.h"

#include <cstdint>

class CAN_dlc
{
public:
	size_t to_len() const;
	void from_len(uint8_t len);

	void set_can_dlc(uint8_t dlc)
	{
		m_dlc = dlc;
	}

	uint8_t get_can_dlc() const
	{
		return m_dlc;
	}

protected:

	uint8_t m_dlc;
};

class STM32_dlc
{
public:
	size_t to_len() const;
	void from_len(uint8_t len);

	CAN_dlc to_can_dlc() const;
	void from_can_dlc(const CAN_dlc& dlc);

protected:

	uint32_t m_dlc;
};