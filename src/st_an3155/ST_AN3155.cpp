#include "ST_AN3155.hpp"

ST_AN3155::ST_AN3155()
{

}

ST_AN3155::~ST_AN3155()
{

}

bool ST_AN3155::ST_AN3155::parse_get()
{
	auto pred_stream_buf_has_n = std::bind(ST_AN3155::stream_buf_has_n, this, std::placeholders::_1);

	std::lock_guard<Mutex_static> lock(m_stream_buf_mutex);
	m_stream_buf_data_avail_mutex.wait(lock, pred_stream_buf_has_n(2));
	return true;
}
bool ST_AN3155::parse_get_version_status()
{
	auto pred_stream_buf_has_n = std::bind(ST_AN3155::stream_buf_has_n, this, std::placeholders::_1);

	std::lock_guard<Mutex_static> lock(m_stream_buf_mutex);
	m_stream_buf_data_avail_mutex.wait(lock, pred_stream_buf_has_n(2));	
	return true;
}
bool ST_AN3155::parse_get_id()
{
	auto pred_stream_buf_has_n = std::bind(ST_AN3155::stream_buf_has_n, this, std::placeholders::_1);

	std::lock_guard<Mutex_static> lock(m_stream_buf_mutex);
	m_stream_buf_data_avail_mutex.wait(lock, pred_stream_buf_has_n(2));	
	return true;
}
bool ST_AN3155::parse_read_mem()
{
	auto pred_stream_buf_has_n = std::bind(ST_AN3155::stream_buf_has_n, this, std::placeholders::_1);

	std::lock_guard<Mutex_static> lock(m_stream_buf_mutex);

	//command & checksum
	m_stream_buf_data_avail_mutex.wait(lock, pred_stream_buf_has_n(2));

	if(is_read_protection_active())
	{
		write_nack();
		return true;
	}

	//address & checksum
	m_stream_buf_data_avail_mutex.wait(lock, pred_stream_buf_has_n(5));

	//length & checksum
	m_stream_buf_data_avail_mutex.wait(lock, pred_stream_buf_has_n(2));

	return true;
}
bool ST_AN3155::parse_go()
{
	return false;
}
bool ST_AN3155::parse_write_mem()
{
	return false;
}
bool ST_AN3155::parse_erase()
{
	return false;
}
bool ST_AN3155::parse_extended_erase()
{
	return false;
}
bool ST_AN3155::parse_write_protect()
{
	return false;
}
bool ST_AN3155::parse_write_unprotect()
{
	return false;
}
bool ST_AN3155::parse_read_protect()
{
	return false;
}
bool ST_AN3155::parse_read_unprotect()
{
	return false;
}

void ST_AN3155::work()
{
	auto pred_stream_buf_not_empty = std::bind(ST_AN3155::stream_buf_not_empty, this);
	auto pred_stream_buf_has_n = std::bind(ST_AN3155::stream_buf_has_n, this, std::placeholders::_1);

	uint8_t cmd = 0;
	{
		std::lock_guard<Mutex_static> lock(m_stream_buf_mutex);
		m_stream_buf_data_avail_mutex.wait(lock, pred_stream_buf_not_empty);

		cmd = m_stream_buf.front();
	}

	switch(cmd)
	{
		case GET:
		{
			parse_get();

			break;
		}
		case GET_VERSION_STATUS:
		{
			break;
		}
		case GET_ID:
		{
			break;
		}
		case READ_MEM:
		{
			break;
		}
		case GO:
		{
			break;
		}
		case WRITE_MEM:
		{
			break;
		}
		case ERASE:
		{
			break;
		}
		case EXTERASE:
		{
			break;
		}
		case WRITE_PROTECT:
		{
			break;
		}
		case WRITE_UNPROTECT:
		{
			break;
		}
		case READ_PROTECT:
		{
			break;
		}
		case READ_UNPROTECT:
		{
			break;
		}
		default:
		{
			std::lock_guard<Mutex_static> lock(m_stream_buf_mutex);
			m_stream_buf.pop_front();
			break;
		}
	}
}