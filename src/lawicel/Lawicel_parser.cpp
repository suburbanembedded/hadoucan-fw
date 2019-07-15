#include "Lawicel_parser.hpp"

#include "CAN_DLC.hpp"

#include "common_util/Byte_util.hpp"

#include "uart1_printf.hpp"

#include <array>
#include <algorithm>

#include <cstring>
#include <cstdio>

bool Lawicel_parser::write_bell()
{
	return write_string("\a");
}
bool Lawicel_parser::write_cr()
{
	return write_string("\r");
}

bool Lawicel_parser::parse_std_id(const char* in_str, uint32_t* const id)
{
	//tiiildd
	std::array<char, 4> id_str;
	id_str.fill(0);
	std::copy_n(in_str+1, 3, id_str.data());

	unsigned int temp_id = 0;
	const int ret = sscanf(id_str.data(), "%x", &temp_id);
	if(ret != 1)
	{
		return false;
	}

	if(temp_id > 0x7FF)
	{
		return false;
	}

	*id = temp_id;

	return true;
}
bool Lawicel_parser::parse_ext_id(const char* in_str, uint32_t* const id)
{
	//Tiiiiiiiildd
	std::array<char, 9> id_str;
	id_str.fill(0);
	std::copy_n(in_str+1, 8, id_str.data());

	unsigned int temp_id = 0;
	const int ret = sscanf(id_str.data(), "%x", &temp_id);
	if(ret != 1)
	{
		return false;
	}

	if(temp_id > 0x1FFFFFFF)
	{
		return false;
	}

	*id = temp_id;

	return true;
}

bool Lawicel_parser::parse_std_dlc(const char dlc_char, uint8_t* const data_len)
{
	CAN_DLC can_dlc;
	if(!can_dlc.from_ascii(dlc_char))
	{
		return false;	
	}

	const uint8_t len = can_dlc.to_len();
	if(len > 8)
	{
		return false;
	}

	*data_len = len;

	return true;
}
bool Lawicel_parser::parse_std_data(const char* data_str, const uint8_t data_len, std::array<uint8_t, 8>* const data)
{
	if(data_len > 8)
	{
		return false;
	}

	const size_t data_str_len = strnlen(data_str, 16);
	if(data_str_len < (data_len*2))
	{
		return false;
	}

	for(size_t i = 0; i < data_len; i++)
	{
		uint8_t d = 0;
		if(!Byte_util::hex_to_byte(data_str+2*i, &d))
		{
			return false;
		}

		(*data)[i] = d;
	}

	return true;
}

bool Lawicel_parser::parse_fd_dlc(const char dlc_char, uint8_t* const data_len)
{
	CAN_DLC can_dlc;
	if(!can_dlc.from_ascii(dlc_char))
	{
		return false;
	}

	const uint8_t len = can_dlc.to_len();
	if(len > 64)
	{
		return false;
	}

	*data_len = len;

	return true;
}
bool Lawicel_parser::parse_fd_data(const char* data_str, const uint8_t data_len, std::array<uint8_t, 64>* const data)
{
	if(data_len > 64)
	{
		return false;
	}

	const size_t data_str_len = strnlen(data_str, 128);
	if(data_str_len < (data_len*2))
	{
		return false;
	}

	for(size_t i = 0; i < data_len; i++)
	{
		uint8_t d = 0;
		if(!Byte_util::hex_to_byte(data_str+2*i, &d))
		{
			return false;
		}

		(*data)[i] = d;
	}

	return true;
}

bool Lawicel_parser::parse_string(const char* in_str)
{
	if(in_str == nullptr)
	{
		return false;
	}

	bool ret = false;
	switch(in_str[0])
	{
		case 'A':
		{
			ret = parse_poll_all(in_str);
			break;
		}
		case 'P':
		{
			ret = parse_poll_one(in_str);
			break;
		}
		case 'S':
		{
			ret = parse_std_baud(in_str);
			break;
		}
		case 's':
		{
			ret = parse_cust_baud(in_str);
			break;
		}
		case 'O':
		{
			ret = parse_open(in_str);
			break;
		}
		case 'L':
		{
			ret = parse_open_listen(in_str);
			break;
		}
		case 'C':
		{
			ret = parse_close(in_str);
			break;
		}
		case 't':
		{
			ret = parse_tx_std(in_str);
			break;
		}
		case 'T':
		{
			ret = parse_tx_ext(in_str);
			break;
		}
		case 'r':
		{
			ret = parse_tx_rtr_std(in_str);
			break;
		}
		case 'R':
		{
			ret = parse_tx_rtr_ext(in_str);
			break;
		}
		case 'd':
		{
			ret = parse_tx_fd_std(in_str);
			break;
		}
		case 'D':
		{
			ret = parse_tx_fd_ext(in_str);
			break;
		}
		case 'F':
		{
			ret = parse_get_flags(in_str);
			break;
		}
		case 'M':
		{
			ret = parse_set_accept_code(in_str);
			break;
		}
		case 'm':
		{
			ret = parse_set_accept_mask(in_str);
			break;
		}
		case 'V':
		{
			ret = parse_get_version(in_str);
			break;
		}
		case 'N':
		{
			ret = parse_get_serial(in_str);
			break;
		}
		case 'X':
		{
			ret = parse_auto_poll(in_str);
			break;
		}
		case 'Z':
		{
			ret = parse_set_timestamp(in_str);
			break;
		}
		case '!':
		{
			ret = parse_extended_cmd(in_str);
			break;
		}
		default:
		{
			uart1_log<128>(LOG_LEVEL::WARN, "Lawicel_parser::parse_string", "no handler for %c", in_str[0]);

			ret = false;
			break;
		}
	}

	return ret;
}

bool Lawicel_parser::parse_std_baud(const char* in_str)
{
	const size_t in_str_len = strnlen(in_str, 3);

	if(in_str_len != 3)
	{
		write_bell();
		return false;
	}
	
	if(in_str[0] != 'S')
	{
		write_bell();
		return false;
	}

	if(in_str[2] != '\r')
	{
		write_bell();
		return false;
	}

	unsigned int baud = 0;
{
	int ret = sscanf(in_str, "S%u\r", &baud);
	if(ret != 1)
	{
		write_bell();
		return false;
	}
}

	if(baud > 8)
	{
		uart1_log<128>(LOG_LEVEL::ERROR, "Lawicel_parser::parse_std_baud", "invalid baud %u", baud);
		write_bell();
		return false;
	}

	if(!handle_std_baud(CAN_NOM_BPS(baud)))
	{
		uart1_log<128>(LOG_LEVEL::ERROR, "Lawicel_parser::parse_std_baud", "handle_std_baud failed");
		write_bell();
		return false;
	}

	write_cr();
	return true;
}
bool Lawicel_parser::parse_cust_baud(const char* in_str)
{
	const size_t in_str_len = strnlen(in_str, 6);

	if(in_str_len != 6)
	{
		write_bell();
		return false;
	}
	
	if(in_str[0] != 's')
	{
		write_bell();
		return false;
	}

	if(in_str[5] != '\r')
	{
		write_bell();
		return false;
	}

	unsigned int b0 = 0;
	unsigned int b1 = 0;

{
	std::array<char, 3> b0_str;
	b0_str.fill(0);
	
	std::copy_n(in_str+1, 2, b0_str.data());

	const int ret = sscanf(b0_str.data(), "%x", &b0);
	if(ret != 1)
	{
		write_bell();
		return false;
	}
}

{
	std::array<char, 3> b1_str;
	b1_str.fill(0);
	std::copy_n(in_str+3, 2, b1_str.data());

	const int ret = sscanf(b1_str.data(), "%x", &b1);
	if(ret != 1)
	{
		write_bell();
		return false;
	}
}
	if(!handle_cust_baud(b0, b1))
	{
		write_bell();
		return false;
	}

	write_cr();
	return true;
}

bool Lawicel_parser::parse_open(const char* in_str)
{
	const int ret = strncmp("O\r", in_str, 2);
	if(ret != 0)
	{
		write_bell();
		return false;
	}

	if(!handle_open())
	{
		write_bell();
		return false;
	}

	m_is_channel_open = true;

	write_cr();
	return true;
}
bool Lawicel_parser::parse_open_listen(const char* in_str)
{
	const int ret = strncmp("L\r", in_str, 2);
	if(ret != 0)
	{
		write_bell();
		return false;
	}

	if(!handle_open_listen())
	{
		write_bell();
		return false;
	}

	write_cr();
	return true;
}
bool Lawicel_parser::parse_close(const char* in_str)
{
	int ret = strncmp("C\r", in_str, 2);
	if(ret != 0)
	{
		write_bell();
		return false;
	}

	if(!handle_close())
	{
		write_bell();
		return false;
	}

	m_is_channel_open = false;

	write_cr();
	return true;
}

bool Lawicel_parser::parse_tx_std(const char* in_str)
{
	const size_t in_str_len = strlen(in_str);

	//tiiil\r
	if(in_str_len < 6)
	{
		write_bell();
		return false;
	}
	
	if(in_str[0] != 't')
	{
		write_bell();
		return false;
	}

	uint32_t id = 0;
	if(!parse_std_id(in_str, &id))
	{
		write_bell();
		return false;
	}

	uint8_t data_len = 0;
	if(!parse_std_dlc(in_str[4], &data_len))
	{
		write_bell();
		return false;
	}

	//verify len
	if(in_str_len != (1U+3U+1U+2U*data_len+1U))
	{
		write_bell();
		return false;
	}

	std::array<uint8_t, 8> data;
	if(!parse_std_data(in_str+5, data_len, &data))
	{
		write_bell();
		return false;
	}

	if(!handle_tx_std(id, data_len, data.data()))
	{
		write_bell();
		return false;
	}

	bool success = false;
	switch(m_poll_mode)
	{
		case POLL_MODE::MANUAL:
		{
			write_cr();
			success = true;
			break;
		}
		case POLL_MODE::AUTO:
		{
			write_string("z\r");
			success = true;
			break;
		}
		default:
		{
			write_bell();
			success = false;
			break;
		}
	}

	return success;
}
bool Lawicel_parser::parse_tx_ext(const char* in_str)
{
	const size_t in_str_len = strlen(in_str);

	//Tiiiiiiiil\r
	if(in_str_len < 11)
	{
		write_bell();
		return false;
	}
	
	if(in_str[0] != 'T')
	{
		write_bell();
		return false;
	}

	uint32_t id = 0;
	if(!parse_ext_id(in_str, &id))
	{
		write_bell();
		return false;
	}

	uint8_t data_len = 0;
	if(!parse_std_dlc(in_str[9], &data_len))
	{
		write_bell();
		return false;
	}

	//verify len
	if(in_str_len != (1U+8U+1U+2U*data_len+1U))
	{
		write_bell();
		return false;
	}

	std::array<uint8_t, 8> data;
	if(!parse_std_data(in_str+10, data_len, &data))
	{
		write_bell();
		return false;
	}

	if(!handle_tx_ext(id, data_len, data.data()))
	{
		write_bell();
		return false;
	}

	bool success = false;
	switch(m_poll_mode)
	{
		case POLL_MODE::MANUAL:
		{
			write_cr();
			success = true;
			break;
		}
		case POLL_MODE::AUTO:
		{
			write_string("Z\r");
			success = true;
			break;
		}
		default:
		{
			write_bell();
			success = false;
			break;
		}
	}
	return success;
}

bool Lawicel_parser::parse_tx_rtr_std(const char* in_str)
{
	const size_t in_str_len = strlen(in_str);

	//riiil\r
	if(in_str_len < 6)
	{
		write_bell();
		return false;
	}
	
	if(in_str[0] != 'r')
	{
		write_bell();
		return false;
	}

	uint32_t id = 0;
	if(!parse_std_id(in_str, &id))
	{
		write_bell();
		return false;
	}

	uint8_t data_len = 0;
	if(!parse_std_dlc(in_str[4], &data_len))
	{
		write_bell();
		return false;
	}

	if(!handle_tx_rtr_std(id, data_len))
	{
		write_bell();
		return false;
	}

	bool success = false;
	switch(m_poll_mode)
	{
		case POLL_MODE::MANUAL:
		{
			write_cr();
			success = true;
			break;
		}
		case POLL_MODE::AUTO:
		{
			write_string("z\r");
			success = true;
			break;
		}
		default:
		{
			write_bell();
			success = false;
			break;
		}
	}

	return success;
}
bool Lawicel_parser::parse_tx_rtr_ext(const char* in_str)
{
	const size_t in_str_len = strlen(in_str);

	//Riiiiiiiil\r
	if(in_str_len < 11)
	{
		write_bell();
		return false;
	}

	if(in_str[0] != 'R')
	{
		write_bell();
		return false;
	}

	uint32_t id = 0;
	if(!parse_ext_id(in_str, &id))
	{
		write_bell();
		return false;
	}

	uint8_t data_len = 0;
	if(!parse_std_dlc(in_str[9], &data_len))
	{
		write_bell();
		return false;
	}

	if(!handle_tx_rtr_ext(id, data_len))
	{
		write_bell();
		return false;
	}

	bool success = false;
	switch(m_poll_mode)
	{
		case POLL_MODE::MANUAL:
		{
			write_cr();
			success = true;
			break;
		}
		case POLL_MODE::AUTO:
		{
			write_string("Z\r");
			success = true;
			break;
		}
		default:
		{
			write_bell();
			success = false;
			break;
		}
	}
	return success;
}

bool Lawicel_parser::parse_tx_fd_std(const char* in_str)
{
	//diiil\r
	const size_t in_str_len = strlen(in_str);

	if(in_str_len < 6)
	{
		uart1_log<128>(LOG_LEVEL::DEBUG, "Lawicel_parser::parse_tx_fd_std", "in_str_len < 6");

		write_bell();
		return false;
	}
	
	if(in_str[0] != 'd')
	{
		uart1_log<128>(LOG_LEVEL::DEBUG, "Lawicel_parser::parse_tx_fd_std", "in_str[0] != 'd'");

		write_bell();
		return false;
	}

	uint32_t id = 0;
	if(!parse_std_id(in_str, &id))
	{
		uart1_log<128>(LOG_LEVEL::DEBUG, "Lawicel_parser::parse_tx_fd_std", "parse_std_id failed");

		write_bell();
		return false;
	}

	//diiil
	uint8_t data_len = 0;
	if(!parse_fd_dlc(in_str[4], &data_len))
	{
		uart1_log<128>(LOG_LEVEL::DEBUG, "Lawicel_parser::parse_tx_fd_std", "parse_fd_dlc failed");

		write_bell();
		return false;
	}

	//verify len
	const size_t expected_len = 1U+3U+1U+2U*data_len+1U;
	if(in_str_len != expected_len)
	{
		uart1_log<128>(LOG_LEVEL::DEBUG, "Lawicel_parser::parse_tx_fd_std", "len verify failed, expected %u, got %u", expected_len, in_str_len);

		write_bell();
		return false;
	}

	std::array<uint8_t, 64> data;
	if(!parse_fd_data(in_str+5, data_len, &data))
	{
		uart1_log<128>(LOG_LEVEL::DEBUG, "Lawicel_parser::parse_tx_fd_std", "parse_fd_data failed");

		write_bell();
		return false;
	}

	if(!handle_tx_fd_std(id, data_len, data.data()))
	{
		uart1_log<128>(LOG_LEVEL::DEBUG, "Lawicel_parser::parse_tx_fd_std", "handle_tx_fd_std failed");

		write_bell();
		return false;
	}

	bool success = false;
	switch(m_poll_mode)
	{
		case POLL_MODE::MANUAL:
		{
			write_cr();
			success = true;
			break;
		}
		case POLL_MODE::AUTO:
		{
			write_string("z\r");
			success = true;
			break;
		}
		default:
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "Lawicel_parser::parse_tx_fd_std", "m_poll_mode invalid");

			write_bell();
			success = false;
			break;
		}
	}

	return success;
}
bool Lawicel_parser::parse_tx_fd_ext(const char* in_str)
{
	//diiiiiiiil\r
	const size_t in_str_len = strlen(in_str);

	if(in_str_len < 11)
	{
		uart1_log<128>(LOG_LEVEL::DEBUG, "Lawicel_parser::parse_tx_fd_ext", "in_str_len < 11");

		write_bell();
		return false;
	}
	
	if(in_str[0] != 'D')
	{
		uart1_log<128>(LOG_LEVEL::DEBUG, "Lawicel_parser::parse_tx_fd_ext", "in_str[0] != 'd'");

		write_bell();
		return false;
	}

	uint32_t id = 0;
	if(!parse_ext_id(in_str, &id))
	{
		uart1_log<128>(LOG_LEVEL::DEBUG, "Lawicel_parser::parse_tx_fd_ext", "parse_std_id failed");

		write_bell();
		return false;
	}

	//diiil
	uint8_t data_len = 0;
	if(!parse_fd_dlc(in_str[9], &data_len))
	{
		uart1_log<128>(LOG_LEVEL::DEBUG, "Lawicel_parser::parse_tx_fd_ext", "parse_fd_dlc failed");

		write_bell();
		return false;
	}

	//verify len
	const size_t expected_len = 1U+8U+1U+2U*data_len+1U;
	if(in_str_len != expected_len)
	{
		uart1_log<128>(LOG_LEVEL::DEBUG, "Lawicel_parser::parse_tx_fd_ext", "len verify failed, expected %u, got %u", expected_len, in_str_len);

		write_bell();
		return false;
	}

	std::array<uint8_t, 64> data;
	if(!parse_fd_data(in_str+10, data_len, &data))
	{
		uart1_log<128>(LOG_LEVEL::DEBUG, "Lawicel_parser::parse_tx_fd_ext", "parse_fd_data failed");

		write_bell();
		return false;
	}

	if(!handle_tx_fd_ext(id, data_len, data.data()))
	{
		uart1_log<128>(LOG_LEVEL::DEBUG, "Lawicel_parser::parse_tx_fd_ext", "handle_tx_fd_ext failed");

		write_bell();
		return false;
	}

	bool success = false;
	switch(m_poll_mode)
	{
		case POLL_MODE::MANUAL:
		{
			write_cr();
			success = true;
			break;
		}
		case POLL_MODE::AUTO:
		{
			write_string("z\r");
			success = true;
			break;
		}
		default:
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "Lawicel_parser::parse_tx_fd_ext", "m_poll_mode invalid");

			write_bell();
			success = false;
			break;
		}
	}

	return success;
}

bool Lawicel_parser::parse_get_flags(const char* in_str)
{
	int ret = strncmp("F\r", in_str, 2);
	if(ret != 0)
	{
		write_bell();
		return false;
	}

	if(!handle_get_flags())
	{
		write_bell();
		return false;
	}

	write_cr();
	return true;
}

bool Lawicel_parser::parse_set_accept_code(const char* in_str)
{
	write_bell();
	return false;
}
bool Lawicel_parser::parse_set_accept_mask(const char* in_str)
{
	write_bell();
	return false;
}

bool Lawicel_parser::parse_get_version(const char* in_str)
{
	const int ret = strncmp("V\r", in_str, 2);
	if(ret != 0)
	{
		write_bell();
		return false;
	}

	std::array<uint8_t, 4> ver;
	if(!handle_get_version(&ver))
	{
		write_bell();
		return false;
	}

	std::array<uint8_t, 7> resp;
	resp[0] = 'V';
	std::copy_n(ver.data(), 4, resp.data()+1);
	resp[5] = '\r';
	resp[6] = '\0';

	write_string((char*)resp.data());

	return true;
}
bool Lawicel_parser::parse_get_serial(const char* in_str)
{
	const int ret = strncmp("N\r", in_str, 2);
	if(ret != 0)
	{
		write_bell();
		return false;
	}

	std::array<uint8_t, 4> sn;
	if(!handle_get_serial(&sn))
	{
		write_bell();
		return false;
	}

	std::array<uint8_t, 7> resp;
	resp[0] = 'N';
	std::copy_n(sn.data(), 4, resp.data()+1);
	resp[5] = '\r';
	resp[6] = '\0';

	write_string((char*)resp.data());

	return true;
}

bool Lawicel_parser::parse_set_timestamp(const char* in_str)
{
	unsigned int timestamp = 0;
	{
		const int ret = sscanf(in_str, "Z%u\r", &timestamp);
		if(ret != 1)
		{
			write_bell();
			return false;
		}
	}

	if(!handle_set_timestamp(timestamp))
	{
		write_bell();
		return false;
	}

	write_cr();
	return true;
}

bool Lawicel_parser::parse_poll_one(const char* in_str)
{
	const int ret = strncmp("P\r", in_str, 2);
	if(ret != 0)
	{
		write_bell();
		return false;
	}

	std::string out_line;
	if(!handle_poll_one(&out_line))
	{
		write_bell();
		return false;
	}

	if(!out_line.empty())
	{
		write_string(out_line.c_str());
	}
	else
	{
		write_cr();
	}

	return true;
}

bool Lawicel_parser::parse_poll_all(const char* in_str)
{
	const int ret = strncmp("A\r", in_str, 2);
	if(ret != 0)
	{
		write_bell();
		return false;
	}

	if(!handle_poll_all())
	{
		write_bell();
		return false;
	}

	write_string("A\r");
	return true;
}

bool Lawicel_parser::parse_auto_poll(const char* in_str)
{
	unsigned int auto_poll = 0;
	{
		const int ret = sscanf(in_str, "X%u\r", &auto_poll);
		if(ret != 1)
		{
			write_bell();
			return false;
		}
	}

	if(auto_poll > 1)
	{
		write_bell();
		return false;
	}

	if(!handle_auto_poll(auto_poll))
	{
		write_bell();
		return false;
	}

	write_cr();
	return true;
}

bool Lawicel_parser::parse_extended_cmd(const char* in_str)
{
	const size_t in_str_len = strlen(in_str);

	const char config_str[] = "!config";
	const size_t config_str_len = strlen(config_str);

	const char printconfig_str[] = "!printconfig\r";
	const size_t printconfig_str_len = strlen(printconfig_str);

	const char table_str[] = "!table";
	const size_t table_str_len = strlen(table_str);

	const char printtable_str[] = "!printtable\r";
	const size_t printtable_str_len = strlen(printtable_str);

	const char defconfig_str[] = "!defconfig\r";
	const size_t defconfig_str_len = strlen(defconfig_str);

	const char bootloader_str[] = "!bootloader\r";
	const size_t bootloader_str_len = strlen(bootloader_str);

	const char serial_str[] = "!serial\r";
	const size_t serial_str_len = strlen(serial_str);

	bool ret = false;

	//TODO: this does not compare substrings as true
	if(strncmp(in_str, config_str, config_str_len) == 0)
	{
		auto it = std::find(in_str, in_str+in_str_len, ':');

		if(it == (in_str+in_str_len))
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "Lawicel_parser::parse_extended_cmd", "Parsing config failed");
		}

		//it points to the ':', start one after
		std::vector<char> config_str(it+1, in_str+in_str_len);
		ret = handle_ext_config(config_str);
	}
	else if(strncmp(in_str, printconfig_str, printconfig_str_len) == 0)
	{
		ret = handle_ext_print_config();
	}
	else if(strncmp(in_str, table_str, table_str_len) == 0)
	{
		auto it = std::find(in_str, in_str+in_str_len, ':');

		if(it == (in_str+in_str_len))
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "Lawicel_parser::parse_extended_cmd", "Parsing table failed");
		}

		std::vector<char> table_str(it, in_str+in_str_len);
		ret = handle_ext_bitrate_table(table_str);
	}
	else if(strncmp(in_str, printtable_str, printtable_str_len) == 0)
	{
		ret = handle_ext_print_bitrate_table();
	}
	else if(strncmp(in_str, defconfig_str, defconfig_str_len) == 0)
	{
		ret = handle_ext_defconfig();
	}
	else if(strncmp(in_str, bootloader_str, bootloader_str_len) == 0)
	{
		uart1_log<128>(LOG_LEVEL::INFO, "Lawicel_parser::parse_extended_cmd", "Rebooting to bootloader");

		ret = handle_ext_bootloader();
	}
	else if(strncmp(in_str, serial_str, serial_str_len) == 0)
	{
		uart1_log<128>(LOG_LEVEL::INFO, "Lawicel_parser::parse_extended_cmd", "Extended serial number");

		ret = handle_ext_serial();
	}
	else
	{
		uart1_log<128>(LOG_LEVEL::WARN, "Lawicel_parser::parse_extended_cmd", "no handler for %s", in_str);

		write_bell();
		return false;
	}

	if(!ret)
	{
		uart1_log<128>(LOG_LEVEL::WARN, "Lawicel_parser::parse_extended_cmd", "Extended command failed");
	}

	write_cr();
	return ret;
}

bool Lawicel_parser::queue_rx_packet(const std::string& packet_str)
{
	bool success = false;
	switch(m_poll_mode)
	{
		case POLL_MODE::MANUAL:
		{
			if((m_rx_packet_buf.size() + packet_str.size()) < MAX_RX_PACKET_BUF_SIZE)
			{
				std::lock_guard<Mutex_static> lock(m_rx_packet_buf_mutex);

				m_rx_packet_buf.insert(m_rx_packet_buf.end(), packet_str.begin(), packet_str.end());
				
				success = true;
			}
			else
			{
				success = false;
			}
			break;
		}
		case POLL_MODE::AUTO:
		{
			write_string(packet_str.c_str());
			success = true;
			break;
		}
		default:
		{
			success = false;
			break;
		}
	}
	return success;
}

bool Lawicel_parser::handle_poll_one(std::string* const out_line)
{
	if(!m_is_channel_open)
	{
		return false;
	}

	if(m_poll_mode != POLL_MODE::MANUAL)
	{
		return false;
	}

	out_line->clear();
	out_line->reserve(MAX_CAN_PACKET_BUF_SIZE + 1);

	{
		std::lock_guard<Mutex_static> lock(m_rx_packet_buf_mutex);

		const auto cr_it = std::find(m_rx_packet_buf.begin(), m_rx_packet_buf.end(), '\r');

		if(cr_it == m_rx_packet_buf.end())
		{
			return true;
		}

		//copy the [begin, \r]
		const auto cr_next_it = std::next(cr_it);
		out_line->assign(m_rx_packet_buf.begin(), cr_next_it);

		//erase the [begin, \r]
		m_rx_packet_buf.erase(m_rx_packet_buf.begin(), cr_next_it);
	}

	return true;
	
}
bool Lawicel_parser::handle_poll_all()
{
	if(!m_is_channel_open)
	{
		return false;
	}

	if(m_poll_mode != POLL_MODE::MANUAL)
	{
		return false;
	}

	std::string out_line;
	out_line.reserve(MAX_CAN_PACKET_BUF_SIZE + 1);

	bool success = true;

	for(;;)
	{
		if(!handle_poll_one(&out_line))
		{
			success = false;
			break;
		}
		if(out_line.empty())
		{
			break;
		}

		if(!write_string(out_line.c_str()))
		{
			success = false;
			break;
		}
	};

	return success;
}

bool Lawicel_parser::handle_auto_poll(const bool enable)
{
	//TODO: race condition, flush queue while new packets come in
	//std::lock_guard<Mutex_static> lock(m_rx_packet_buf_mutex);

	if(enable)
	{
		uart1_log<128>(LOG_LEVEL::INFO, "Lawicel_parser::handle_auto_poll", "POLL_MODE::AUTO");

		m_poll_mode = POLL_MODE::AUTO;
	}
	else
	{
		uart1_log<128>(LOG_LEVEL::INFO, "Lawicel_parser::handle_auto_poll", "POLL_MODE::MANUAL");

		m_poll_mode = POLL_MODE::MANUAL;
	}

	return true;
}
