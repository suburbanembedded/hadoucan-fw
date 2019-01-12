#pragma once

#include "interface/UART_base.hpp"

#include "freertos_cpp_util/Mutex_static.hpp"
#include "freertos_cpp_util/Condition_variable.hpp"

class ST_AN3155
{
public:

	ST_AN3155();
	virtual ~ST_AN3155();

	//send version and commands
	virtual bool handle_get(uint8_t* const out_ver, std::vector<uint8_t>* const out_cmds) = 0;
	virtual bool handle_get_version_status(uint8_t* const out_ver, uint8_t* const out_b1, uint8_t* const out_b2) = 0;
	virtual bool handle_get_id(uint8_t* const out_id) = 0;
	virtual bool handle_read_mem(const uint32_t addr, std::vector<uint8_t>* const out_buf) = 0;
	virtual bool handle_go(const uint32_t addr) = 0;
	virtual bool handle_write_mem(const uint32_t addr, const std::vector<uint8_t>& buf) = 0;
	virtual bool handle_erase(const uint8_t num_pages, const std::vector<uint8_t>& page_numbers) = 0;
	virtual bool handle_extended_erase(const uint16_t num_pages, const std::vector<uint16_t>& page_numbers) = 0;
	virtual bool handle_write_protect(const uint16_t num_sectors, const std::vector<uint8_t>& sectors_numbers) = 0;
	virtual bool handle_write_unprotect() = 0;
	virtual bool handle_read_protect() = 0;
	virtual bool handle_read_unprotect() = 0;

	virtual bool is_read_protection_active() = 0;
	virtual bool is_write_protection_active() = 0;

	void insert_data(const uint8_t data[], const size_t len);

	void write_data(const uint8_t data[], const size_t len);

	void write_nack();

protected:

	void work();

	bool parse_get();
	bool parse_get_version_status();
	bool parse_get_id();
	bool parse_read_mem();
	bool parse_go();
	bool parse_write_mem();
	bool parse_erase();
	bool parse_extended_erase();
	bool parse_write_protect();
	bool parse_write_unprotect();
	bool parse_read_protect();
	bool parse_read_unprotect();

	enum class COMMAND
	{
		GET = 0x00,
		GET_VERSION_STATUS = 0x01,
		GET_ID = 0x02,
		READ_MEM = 0x11,
		GO = 0x21,
		WRITE_MEM = 0x31,
		ERASE = 0x43,
		EXTERASE = 0x44,
		WRITE_PROTECT = 0x63,
		WRITE_UNPROTECT = 0x73,
		READ_PROTECT = 0x82,
		READ_UNPROTECT = 0x92
	};

	//0x31 is latest published
	constexpr uint8_t VERSION = 0x31;

	constexpr uint8_t SYNC = 0x79;
	constexpr uint8_t ACK = 0x79;
	constexpr uint8_t ACK = 0x1F;

	bool stream_buf_not_empty() const
	{
		return !m_stream_buf.empty();
	}

	bool stream_buf_has_n(size_t n) const
	{
		return m_stream_buf.size() >= n;		
	}

	std::dequeue m_stream_buf;
	Mutex_static m_stream_buf_mutex;
	Condition_variable m_stream_buf_data_avail_mutex;

	std::vector<uint8_t> m_packet_buf;

	UART_base* m_uart;
};
