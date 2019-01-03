#pragma once

#include <array>
#include <functional>

#include <cstdint>

class Lawicel_parser
{
	public:

	typedef std::function<bool (const char* str)> WriteStringCallback;

	Lawicel_parser()
	{
		m_write_str_func = nullptr;
	}

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

		return m_write_str_func(str);
	}

	bool parse_string(const char* in_str);

	virtual bool handle_std_baud(const uint8_t baud) = 0;
	virtual bool handle_cust_baud(const uint8_t b0, const uint8_t b1) = 0;
	virtual bool handle_open() = 0;
	virtual bool handle_close() = 0;
	virtual bool handle_tx_std(const uint32_t id, const uint8_t dlc, const uint8_t* data) = 0;
	virtual bool handle_tx_ext(const uint32_t id, const uint8_t dlc, const uint8_t* data) = 0;
	virtual bool handle_tx_rtr_std(const uint32_t id, const uint8_t dlc, const uint8_t* data) = 0;
	virtual bool handle_tx_rtr_ext(const uint32_t id, const uint8_t dlc, const uint8_t* data) = 0;
	virtual bool handle_get_flags() = 0;
	virtual bool handle_set_accept_code(const uint32_t code) = 0;
	virtual bool handle_set_accept_mask(const uint32_t mask) = 0;
	virtual bool handle_get_version(std::array<uint8_t, 4>* ver) = 0;
	virtual bool handle_get_serial(std::array<uint8_t, 4>* sn) = 0;
	virtual bool handle_set_timestamp(bool enable) = 0;

	protected:

	WriteStringCallback m_write_str_func;

	bool parse_std_baud(const char* in_str);
	bool parse_cust_baud(const char* in_str);

	bool parse_open(const char* in_str);
	bool parse_close(const char* in_str);

	bool parse_tx_std(const char* in_str);
	bool parse_tx_ext(const char* in_str);
	
	bool parse_tx_rtr_std(const char* in_str);
	bool parse_tx_rtr_ext(const char* in_str);

	bool parse_get_flags(const char* in_str);

	bool parse_set_accept_code(const char* in_str);
	bool parse_set_accept_mask(const char* in_str);

	bool parse_get_version(const char* in_str);
	bool parse_get_serial(const char* in_str);
	bool parse_set_timestamp(const char* in_str);

	bool parse_std_id(const char* in_str, uint32_t* const id);
	bool parse_ext_id(const char* in_str, uint32_t* const id);

	bool parse_std_dlc(const char* dlc_str, uint8_t* const dlc);
	bool parse_std_data(const char* data_str, const uint8_t dlc, std::array<uint8_t, 8>* const data);

	// bool parse_fd_dlc(const char* in_str, uint8_t* const dlc);
	// bool parse_fd_data(const char* in_str, const uint8_t dlc, std::array<uint8_t, 64>* const data);

	bool write_bell();
	bool write_cr();
};
