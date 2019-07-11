#include "CAN_USB_app.hpp"

#include "uart1_printf.hpp"

#include <algorithm>

CAN_USB_app::CAN_USB_app()
{

}

CAN_USB_app::~CAN_USB_app()
{

}

bool CAN_USB_app::load_config()
{
	tinyxml2::XMLDocument config_doc;

	{
		std::lock_guard<Mutex_static_recursive> lock(m_mutex);

		if(!load_xml_file(m_fs.get_fs(), "config.xml", &config_doc))
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "Opening config.xml failed");
			return false;
		}

		if(!m_config.from_xml(config_doc))
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "Parsing config.xml failed");
			return false;
		}
	}

	return true;
}

bool CAN_USB_app::load_bitrate_table()
{
	tinyxml2::XMLDocument table_doc;

	{
		std::lock_guard<Mutex_static_recursive> lock(m_mutex);

		if(!load_xml_file(m_fs.get_fs(), "table.xml", &table_doc))
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "Opening table.xml failed");
		}

		if(!m_bitrate_tables.from_xml(table_doc))
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "Parsing table.xml failed");
			return false;
		}
	}

	return true;
}

bool CAN_USB_app::write_config(const CAN_USB_app_config& config)
{
	tinyxml2::XMLDocument config_doc;
	if(!config.to_xml(&config_doc))
	{
		uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app::write_config", "config to xml failed");
		return false;
	}

	{
		std::lock_guard<Mutex_static_recursive> lock(m_mutex);
	
		if(!write_xml_file(m_fs.get_fs(), "config.xml", config_doc))
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app::write_config", "Writing config.xml failed");
			return false;
		}

		//update master in-mem copy
		m_config = config;

		//update fdcan's copy
		//it will be re-examined on next call to open
		//this is not a race condition, since this function is called from the same context as open
		get_can_tx().set_config(config.get_config());
	}

	return true;
}
bool CAN_USB_app::write_default_config()
{
	CAN_USB_app_config default_config;
	default_config.set_defualt();

	return write_config(default_config);
}
bool CAN_USB_app::write_bitrate_table(const CAN_USB_app_bitrate_table& table)
{
	tinyxml2::XMLDocument table_doc;
	if(!table.to_xml(&table_doc))
	{
		return false;
	}

	{
		std::lock_guard<Mutex_static_recursive> lock(m_mutex);

		if(!write_xml_file(m_fs.get_fs(), "table.xml", table_doc))
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "Writing table.xml failed");
			return false;
		}

		m_bitrate_tables = table;
	}

	return true;
}
bool CAN_USB_app::write_default_bitrate_table()
{
	CAN_USB_app_bitrate_table default_table;
	default_table.set_defualt();

	return write_bitrate_table(default_table);
}

bool CAN_USB_app::load_xml_file(spiffs* const fs, const char* name, tinyxml2::XMLDocument* const out_xml)
{
	spiffs_file fd = SPIFFS_open(fs, name, SPIFFS_RDONLY, 0);
	if(fd < 0)
	{
		uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "Opening %s failed: %" PRId32, name, SPIFFS_errno(fs));
		return false;
	}

	spiffs_stat stat;
	if(SPIFFS_fstat(fs, fd, &stat) < 0)
	{
		uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "Getting size of %s: %" PRId32, name, SPIFFS_errno(fs));
		return false;
	}

	std::vector<char> data;
	data.resize(stat.size);
	if(SPIFFS_read(fs, fd, data.data(), data.size()) < 0)
	{
		uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "Reading %s failed: %" PRId32, name, SPIFFS_errno(fs));
		return false;
	}

	if(SPIFFS_close(fs, fd) < 0)
	{
		uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "Closing %s failed: %" PRId32, name, SPIFFS_errno(fs));
		return false;
	}

	out_xml->Clear();
	tinyxml2::XMLError err = out_xml->Parse(data.data(), data.size());
	if(err != tinyxml2::XML_SUCCESS)
	{
		uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "Parsing %s failed: %" PRId32, name, err);
		return false;
	}

	return true;
}

bool CAN_USB_app::write_xml_file(spiffs* const fs, const char* name, const tinyxml2::XMLDocument& xml)
{
	tinyxml2::XMLPrinter xml_printer(nullptr, false, 0);
	xml.Print(&xml_printer);

	const char* doc_str = xml_printer.CStr();

	//Includes trailing null, should never return 0
	int xml_printer_len = xml_printer.CStrSize();
	if(xml_printer_len < 1)
	{
		uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "xml print error");
		return false;
	}

	//Remove trailing null
	const size_t doc_str_len = xml_printer_len - 1;

	for(size_t i = 0; i < doc_str_len; i++)
	{
		uart1_printf<16>("%c", doc_str[i]);
	}

	uart1_log<128>(LOG_LEVEL::INFO, "CAN_USB_app", "Writing %s", name);
	spiffs_file fd = SPIFFS_open(fs, name, SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_RDWR, 0);
	if(fd < 0)
	{
		uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "Opening %s failed: %" PRId32, name, SPIFFS_errno(fs));
		return false;
	}

	if(SPIFFS_write(fs, fd, const_cast<char*>(doc_str), doc_str_len) < 0)
	{
		uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "Writing %s failed: %" PRId32, name, SPIFFS_errno(fs));
		return false;
	}

	if(SPIFFS_close(fs, fd) < 0)
	{
		uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "Closing %s failed: %" PRId32, name, SPIFFS_errno(fs));
		return false;
	}
	uart1_log<128>(LOG_LEVEL::INFO, "CAN_USB_app", "Write %s success", name);

	return true;
}

void CAN_USB_app::get_unique_id(std::array<uint32_t, 3>* id)
{
	volatile uint32_t* addr = reinterpret_cast<uint32_t*>(0x1FF1E800);

	std::copy_n(addr, 3, id->data());
}

void CAN_USB_app::get_unique_id_str(std::array<char, 25>* id_str)
{
	//0x012345670123456701234567
	std::array<uint32_t, 3> id;
	get_unique_id(&id);

	snprintf(id_str->data(), id_str->size(), "%08" PRIX32 "%08" PRIX32 "%08" PRIX32, id[0], id[1], id[2]);
}