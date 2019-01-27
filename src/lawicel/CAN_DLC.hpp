#pragma once

#include <cstdint>
#include <cstddef>

class CAN_DLC
{
public:
	size_t to_len() const;
	bool from_len(uint8_t len);

	bool from_ascii(char c);

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
