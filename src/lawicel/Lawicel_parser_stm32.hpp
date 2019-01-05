#pragma once

#include "Lawicel_parser.hpp"

#include "../STM32_fdcan_tx.hpp"

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_fdcan.h"

class Lawicel_parser_stm32 : public Lawicel_parser
{
	public:

	Lawicel_parser_stm32()
	{
		m_fdcan = nullptr;
	}

	void set_can(STM32_fdcan_tx* const fdcan)
	{
		m_fdcan = fdcan;
	}

	bool handle_std_baud(const uint8_t baud) override;
	bool handle_cust_baud(const uint8_t b0, const uint8_t b1) override;
	bool handle_open() override;
	bool handle_open_listen() override;
	bool handle_close() override;
	bool handle_tx_std(const uint32_t id, const uint8_t dlc, const uint8_t* data) override;
	bool handle_tx_ext(const uint32_t id, const uint8_t dlc, const uint8_t* data) override;
	bool handle_tx_rtr_std(const uint32_t id, const uint8_t dlc) override;
	bool handle_tx_rtr_ext(const uint32_t id, const uint8_t dlc) override;
	bool handle_get_flags() override;
	bool handle_set_accept_code(const uint32_t code) override;
	bool handle_set_accept_mask(const uint32_t mask) override;
	bool handle_get_version(std::array<uint8_t, 4>* const ver) override;
	bool handle_get_serial(std::array<uint8_t, 4>* const sn) override;
	bool handle_set_timestamp(const bool enable) override;

	bool handle_poll_one() override;
	bool handle_poll_all() override;

	bool handle_auto_poll(const bool enable) override;

	protected:

	STM32_fdcan_tx* m_fdcan;

};
