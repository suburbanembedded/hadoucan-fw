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

	bool handle_std_baud(const CAN_NOM_BPS baud) override;
	bool handle_cust_baud(const uint8_t BTR0, const uint8_t BTR1) override;
	bool handle_open() override;
	bool handle_open_listen() override;
	bool handle_close() override;
	bool handle_tx_std(const uint32_t id, const uint8_t data_len, const uint8_t* data) override;
	bool handle_tx_ext(const uint32_t id, const uint8_t data_len, const uint8_t* data) override;
	
	bool handle_tx_rtr_std(const uint32_t id, const uint8_t data_len) override;
	bool handle_tx_rtr_ext(const uint32_t id, const uint8_t data_len) override;
	
	bool handle_tx_fd_std(const uint32_t id, const uint8_t data_len, const uint8_t* data) override;
	bool handle_tx_fd_ext(const uint32_t id, const uint8_t data_len, const uint8_t* data) override;

	bool handle_tx_fd_std_brs(const uint32_t id, const uint8_t data_len, const uint8_t* data) override;
	bool handle_tx_fd_ext_brs(const uint32_t id, const uint8_t data_len, const uint8_t* data) override;

	bool handle_get_flags() override;
	bool handle_set_filter_mode(const char mode) override;
	bool handle_set_accept_code(const uint32_t code) override;
	bool handle_set_accept_mask(const uint32_t mask) override;
	bool handle_get_version(std::array<uint8_t, 4>* const ver) override;
	bool handle_get_serial(std::array<uint8_t, 4>* const sn) override;
	bool handle_set_timestamp(const bool enable) override;
	bool handle_set_autostartup(const bool enable) override;

	bool handle_ext_config(const std::vector<char>& config_str) override;
	bool handle_ext_print_config() override;
	bool handle_ext_bitrate_table(const std::vector<char>& table_str) override;
	bool handle_ext_print_bitrate_table() override;
	bool handle_ext_defconfig() override;
	bool handle_ext_bootloader() override;
	bool handle_ext_serial() override;
	bool handle_ext_version() override;

	protected:

	STM32_fdcan_tx* m_fdcan;

};
