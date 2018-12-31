#pragma once

#include "Lawicel_parser.hpp"

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_fdcan.h"

class Lawicel_parser_stm32 : public Lawicel_parser
{
	public:

	bool write_string(const char* out_str) override;

	bool handle_std_baud(const uint8_t baud) override;
	bool handle_cust_baud(const uint8_t b0, const uint8_t b1) override;
	bool handle_open() override;
	bool handle_close() override;
	bool handle_tx_std(const uint32_t id, const uint8_t dlc, const uint8_t* data) override;
	bool handle_tx_ext(const uint32_t id, const uint8_t dlc, const uint8_t* data) override;
	bool handle_tx_rtr_std(const uint32_t id, const uint8_t dlc, const uint8_t* data) override;
	bool handle_tx_rtr_ext(const uint32_t id, const uint8_t dlc, const uint8_t* data) override;
	bool handle_get_flags() override;
	bool handle_set_accept_code(const uint32_t code) override;
	bool handle_set_accept_mask(const uint32_t mask) override;
	bool handle_get_version(std::array<uint8_t, 4>* ver) override;
	bool handle_get_serial(std::array<uint8_t, 4>* sn) override;
	bool handle_set_timestamp(bool enable) override;

	protected:

	FDCAN_HandleTypeDef* m_fdcan;

};