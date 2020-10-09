#pragma once

#include "../CAN_USB_app_config.hpp"

#include "freertos_cpp_util/Mutex_static.hpp"

#include <array>
#include <vector>
#include <deque>
#include <functional>
#include <mutex>

#include <cstdint>

class Lawicel_parser
{
	public:

	typedef std::function<bool (const char* str)> WriteStringCallback;

	enum class CAN_NOM_BPS
	{
		bps_10k  = 0,
		bps_20k  = 1,
		bps_50k  = 2,
		bps_100k = 3,
		bps_125k = 4,
		bps_250k = 5,
		bps_500k = 6,
		bps_800k = 7,
		bps_1M   = 8
	};

	enum class CAN_BRS_BPS
	{
		bps_NO_CHANGE,
		bps_2M,
		bps_4M,
		bps_8M,
		bps_12M
	};

	enum class POLL_MODE
	{
		MANUAL,
		AUTO
	};

	Lawicel_parser()
	{
		m_write_str_func = nullptr;

		m_is_channel_open = false;
		m_poll_mode = POLL_MODE::AUTO;
	}

	virtual ~Lawicel_parser()
	{

	}

	bool queue_rx_packet(const std::string& packet_str);

	void set_write_string_func(WriteStringCallback func)
	{
		m_write_str_func = func;
	}

	bool write_string(const char* str)
	{
		if(m_write_str_func == nullptr)
		{
			return false;
		}

		std::lock_guard<Mutex_static> lock(m_write_string_mutex);

		return m_write_str_func(str);
	}

	bool parse_string(const char* in_str);

	virtual bool handle_std_baud(const CAN_NOM_BPS baud) = 0;
	virtual bool handle_cust_baud(const uint8_t b0, const uint8_t b1) = 0;
	
	virtual bool handle_open() = 0;
	virtual bool handle_open_listen() = 0;
	virtual bool handle_close() = 0;
	
	virtual bool handle_tx_std(const uint32_t id, const uint8_t data_len, const uint8_t* data) = 0;
	virtual bool handle_tx_ext(const uint32_t id, const uint8_t data_len, const uint8_t* data) = 0;
	
	virtual bool handle_tx_rtr_std(const uint32_t id, const uint8_t data_len) = 0;
	virtual bool handle_tx_rtr_ext(const uint32_t id, const uint8_t data_len) = 0;
	
	virtual bool handle_tx_fd_std(const uint32_t id, const uint8_t data_len, const uint8_t* data) = 0;
	virtual bool handle_tx_fd_ext(const uint32_t id, const uint8_t data_len, const uint8_t* data) = 0;

	virtual bool handle_tx_fd_std_brs(const uint32_t id, const uint8_t data_len, const uint8_t* data) = 0;
	virtual bool handle_tx_fd_ext_brs(const uint32_t id, const uint8_t data_len, const uint8_t* data) = 0;

	virtual bool handle_get_flags() = 0;
	
	virtual bool handle_set_filter_mode(const char mode) = 0;
	virtual bool handle_set_accept_code(const uint32_t code) = 0;
	virtual bool handle_set_accept_mask(const uint32_t mask) = 0;
	
	virtual bool handle_get_version(std::array<uint8_t, 4>* const ver) = 0;
	virtual bool handle_get_serial(std::array<uint8_t, 4>* const sn) = 0;
	
	virtual bool handle_set_timestamp(const bool enable) = 0;

	virtual bool handle_set_autostartup(const bool enable) = 0;

	virtual bool handle_ext_config(const std::vector<char>& config) = 0;
	virtual bool handle_ext_print_config() = 0;
	virtual bool handle_ext_bitrate_table(const std::vector<char>& table) = 0;
	virtual bool handle_ext_print_bitrate_table() = 0;
	virtual bool handle_ext_defconfig() = 0;
	virtual bool handle_ext_bootloader() = 0;
	virtual bool handle_ext_serial() = 0;
	virtual bool handle_ext_version() = 0;

	protected:

	bool handle_poll_one(std::string* const out_line);
	bool handle_poll_all();

	bool handle_auto_poll(const bool enable);

	bool parse_std_baud(const char* in_str);
	bool parse_cust_baud(const char* in_str);

	bool parse_open(const char* in_str);
	bool parse_open_listen(const char* in_str);
	bool parse_close(const char* in_str);

	bool parse_tx_std(const char* in_str);
	bool parse_tx_ext(const char* in_str);
	
	bool parse_tx_rtr_std(const char* in_str);
	bool parse_tx_rtr_ext(const char* in_str);

	bool parse_tx_fd_std(const char* in_str);
	bool parse_tx_fd_ext(const char* in_str);
	
	bool parse_tx_fd_std_brs(const char* in_str);
	bool parse_tx_fd_ext_brs(const char* in_str);

	bool parse_get_flags(const char* in_str);

	bool parse_set_filter_mode(const char* in_str);
	bool parse_set_accept_code(const char* in_str);
	bool parse_set_accept_mask(const char* in_str);

	bool parse_get_version(const char* in_str);
	bool parse_get_serial(const char* in_str);
	bool parse_set_timestamp(const char* in_str);

	bool parse_set_autostartup(const char* in_str);

	bool parse_poll_one(const char* in_str);
	bool parse_poll_all(const char* in_str);

	bool parse_auto_poll(const char* in_str);

	bool parse_std_id(const char* in_str, uint32_t* const id);
	bool parse_ext_id(const char* in_str, uint32_t* const id);

	bool parse_std_dlc(const char dlc_char, uint8_t* const data_len);
	bool parse_std_data(const char* data_str, const uint8_t data_len, std::array<uint8_t, 8>* const data);

	bool parse_fd_dlc(const char dlc_char, uint8_t* const data_len);
	bool parse_fd_data(const char* data_str, const uint8_t data_len, std::array<uint8_t, 64>* const data);

	bool parse_extended_cmd(const char* in_str);

	bool write_bell();
	bool write_cr();

	WriteStringCallback m_write_str_func;

	bool m_is_channel_open;

	POLL_MODE m_poll_mode;

	Mutex_static m_write_string_mutex;

	Mutex_static m_rx_packet_buf_mutex;
	std::deque<char> m_rx_packet_buf;
	constexpr static size_t MAX_CAN_PACKET_BUF_SIZE = 1+8+128+1;
	constexpr static size_t MAX_RX_PACKET_BUF_SIZE = 64 * MAX_CAN_PACKET_BUF_SIZE;
};
