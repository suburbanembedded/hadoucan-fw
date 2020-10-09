#include "CAN_USB_app_config.hpp"

#include "freertos_cpp_util/logging/Global_logger.hpp"

#include "common_util/Byte_util.hpp"

void CAN_USB_app_config::set_defualt()
{	
	m_config.config_version = 0;

	m_config.host_protocol = HOST_PROTOCOL::LAWICEL_CAN232;

	m_config.autopoll = false;
	m_config.listen_only = false;

	m_config.auto_startup = false;

	m_config.timesync_mode = TIMESYNC_MODE::SLAVE;

	m_config.slope_ctrl = SLOPE_CONTROL::AUTO;

	m_config.timestamp_enable     = false;
	m_config.timestamp_prescaler = 2000;
	m_config.timestamp_period    = 50000;

	m_config.tx_delay_comp_enable = false;
	m_config.tx_delay_comp_offset = 5;
	m_config.tx_delay_comp_filter_window = 0;

	m_config.can_clock       = 24000000;
	m_config.bitrate_nominal = 500000;
	m_config.bitrate_data    = 2000000;

	m_config.protocol_ext_id = true;
	m_config.protocol_fd     = true;
	m_config.protocol_brs    = true;
	m_config.protocol_fd_iso = true;

	m_config.sja1000_filter.set_default();

	m_config.log_level = freertos_util::logging::LOG_LEVEL::INFO;
	m_config.uart_baud = 921600U;
}

bool CAN_USB_app_config::to_xml(tinyxml2::XMLDocument* const config_doc) const
{
	// freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();
	// using freertos_util::logging::LOG_LEVEL;

	config_doc->Clear();

	{
		tinyxml2::XMLDeclaration* decl = config_doc->NewDeclaration("xml version=\"1.0\" standalone=\"yes\" encoding=\"UTF-8\"");
		config_doc->InsertFirstChild(decl);
	}

	tinyxml2::XMLElement* config_doc_root = config_doc->NewElement("config");
	config_doc_root->SetAttribute("version", m_config.config_version);
	config_doc->InsertEndChild(config_doc_root);

	tinyxml2::XMLElement* node = nullptr;

	{
		node = config_doc->NewElement("host_protocol");
		switch(m_config.host_protocol)
		{
			case HOST_PROTOCOL::LAWICEL_CAN232:
			{
				node->SetText("lawicel_can232");
				break;
			}
			default:
			{
				node->SetText("lawicel_can232");
				break;
			}
		}
		config_doc->InsertEndChild(config_doc_root);
	}

	//General Config Settings
	node = config_doc->NewElement("autopoll");
	node->SetText(m_config.autopoll);
	config_doc_root->InsertEndChild(node);

	node = config_doc->NewElement("listen_only");
	node->SetText(m_config.listen_only);
	config_doc_root->InsertEndChild(node);

	node = config_doc->NewElement("auto_startup");
	node->SetText(m_config.auto_startup);
	config_doc_root->InsertEndChild(node);
	
	{
		node = config_doc->NewElement("timesync_mode");
	
		switch(m_config.timesync_mode)
		{
			case TIMESYNC_MODE::MASTER:
			{
				node->SetText("master");
				break;
			}
			case TIMESYNC_MODE::SLAVE:
			{
				node->SetText("slave");
				break;
			}
			default:
			{
				node->SetText("slave");
				break;
			}
		}
	
		config_doc_root->InsertEndChild(node);
	}
	
	{
		tinyxml2::XMLComment* comment = config_doc->NewComment("Set slope_ctrl to auto to enable fast slope control if bit rates will be higher than 1 Mbps");
		config_doc_root->InsertEndChild(comment);
		comment = config_doc->NewComment("Set slope_ctrl to fast for CAN FD BRS or maybe for ~1MBps");
		config_doc_root->InsertEndChild(comment);
		comment = config_doc->NewComment("Otherwise set to slow for reduced EMI and better tolerance of lower quality wiring");
		config_doc_root->InsertEndChild(comment);
		
		node = config_doc->NewElement("slope_ctrl");
		switch(m_config.slope_ctrl)
		{
			case SLOPE_CONTROL::SLOW:
			{
				node->SetText("slow");
				break;
			}
			case SLOPE_CONTROL::FAST:
			{
				node->SetText("fast");
				break;
			}
			case SLOPE_CONTROL::AUTO:
			{
				node->SetText("auto");
				break;
			}
			default:
			{
				node->SetText("auto");
				break;
			}
		}
		config_doc_root->InsertEndChild(node);
	}

	{
		tinyxml2::XMLComment* comment = config_doc->NewComment("Enable tx_delay_comp to turn on loop delay compensation");
		config_doc_root->InsertEndChild(comment);
		comment = config_doc->NewComment("Parameters offset and filter_window are in units of mtq, which is 1/fdcan_tq_ck (configured by the clock parameter)");
		config_doc_root->InsertEndChild(comment);
		comment = config_doc->NewComment("Propagation delay on the ADM3055E is about 150ns");
		config_doc_root->InsertEndChild(comment);

		tinyxml2::XMLElement* tx_delay_comp = config_doc->NewElement("tx_delay_comp");
		config_doc_root->InsertEndChild(tx_delay_comp);
		
		node = config_doc->NewElement("enable");
		node->SetText(m_config.tx_delay_comp_enable);
		tx_delay_comp->InsertEndChild(node);

		node = config_doc->NewElement("offset");
		node->SetText(m_config.tx_delay_comp_offset);
		tx_delay_comp->InsertEndChild(node);

		node = config_doc->NewElement("filter_window");
		node->SetText(m_config.tx_delay_comp_filter_window);
		tx_delay_comp->InsertEndChild(node);
	}

	{
		tinyxml2::XMLElement* timestamp = config_doc->NewElement("timestamp");
		config_doc_root->InsertEndChild(timestamp);

		node = config_doc->NewElement("enable");
		node->SetText(m_config.timestamp_enable);
		timestamp->InsertEndChild(node);

		node = config_doc->NewElement("prescaler");
		node->SetText(m_config.timestamp_prescaler);
		timestamp->InsertEndChild(node);

		node = config_doc->NewElement("period");
		node->SetText(m_config.timestamp_period);
		timestamp->InsertEndChild(node);
	}

	{
		tinyxml2::XMLComment* comment = config_doc->NewComment("clock may only be 24000000, 60000000, or 80000000");
		config_doc_root->InsertEndChild(comment);

		node = config_doc->NewElement("clock");
		node->SetText(m_config.can_clock);
		config_doc_root->InsertEndChild(node);
	}

	{
		tinyxml2::XMLElement* bitrate = config_doc->NewElement("bitrate");
		config_doc_root->InsertEndChild(bitrate);

		node = config_doc->NewElement("nominal");
		node->SetText(m_config.bitrate_nominal);
		bitrate->InsertEndChild(node);

		node = config_doc->NewElement("data");
		node->SetText(m_config.bitrate_data);
		bitrate->InsertEndChild(node);
	}
	
	{
		tinyxml2::XMLElement* protocol = config_doc->NewElement("protocol");
		config_doc_root->InsertEndChild(protocol);

		node = config_doc->NewElement("ext_id");
		node->SetText(m_config.protocol_ext_id);
		protocol->InsertEndChild(node);

		node = config_doc->NewElement("fd");
		node->SetText(m_config.protocol_fd);
		protocol->InsertEndChild(node);

		node = config_doc->NewElement("brs");
		node->SetText(m_config.protocol_brs);
		protocol->InsertEndChild(node);

		node = config_doc->NewElement("fd_iso");
		node->SetText(m_config.protocol_fd_iso);
		protocol->InsertEndChild(node);
	}

	{
		tinyxml2::XMLElement* filter = config_doc->NewElement("filter");
		filter->SetAttribute("type", "sja1000");
		config_doc_root->InsertEndChild(filter);

		node = config_doc->NewElement("enable");
		node->SetText(m_config.sja1000_filter.enable);
		filter->InsertEndChild(node);

		node = config_doc->NewElement("mode");
		switch(m_config.sja1000_filter.mode)
		{
			case SJA1000_filter::FILTER_MODE::DUAL:
			{
				node->SetText("DUAL");
				break;
			}
			case SJA1000_filter::FILTER_MODE::SINGLE:
			{
				node->SetText("SINGLE");
				break;
			}
			default:
			{
				node->SetText("DUAL");
				break;
			}
		}
		filter->InsertEndChild(node);

		std::array<char, 9> str;
		Byte_util::u32_to_hex_str(m_config.sja1000_filter.accept_code, &str);
		node = config_doc->NewElement("accept_code");
		node->SetText(str.data());
		filter->InsertEndChild(node);

		Byte_util::u32_to_hex_str(m_config.sja1000_filter.accept_mask, &str);
		node = config_doc->NewElement("accept_mask");
		node->SetText(str.data());
		filter->InsertEndChild(node);
	}

	{
		tinyxml2::XMLElement* debug = config_doc->NewElement("debug");
		config_doc_root->InsertEndChild(debug);

		tinyxml2::XMLComment* comment = config_doc->NewComment("log_level may be TRACE, DEBUG, INFO, WARN, ERROR, FATAL");
		debug->InsertEndChild(comment);

		node = config_doc->NewElement("log_level");
		node->SetText(freertos_util::logging::LOG_LEVEL_to_str(m_config.log_level));
		debug->InsertEndChild(node);

		node = config_doc->NewElement("uart_baud");
		node->SetText(m_config.uart_baud);
		debug->InsertEndChild(node);
	}

	return true;
}

bool CAN_USB_app_config::from_xml(const tinyxml2::XMLDocument& config_doc)
{
	freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();
	using freertos_util::logging::LOG_LEVEL;

	const tinyxml2::XMLElement* const config_root = config_doc.FirstChildElement("config");
	if(config_root == nullptr)
	{
		logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element config");
		return false;
	}

	if(config_root->QueryUnsignedAttribute("version", &m_config.config_version) != tinyxml2::XML_SUCCESS)
	{
		logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find attr ::version");
		return false;
	}

	{
		char const* host_protocol_str;
		if(!get_str_text(config_root, "host_protocol", &host_protocol_str))
		{
			logger->log(LOG_LEVEL::WARN, "CAN_USB_app", "config.xml: could not find element host_protocol, defaulting to lawicel_can232");
			m_config.host_protocol = HOST_PROTOCOL::LAWICEL_CAN232;
		}
		else
		{
			if(strncasecmp(host_protocol_str, "lawicel_can232", 14) == 0)
			{
				m_config.host_protocol = HOST_PROTOCOL::LAWICEL_CAN232;
			}	
			else
			{
				logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not parse element host_protocol");
				return false;
			}
		}
	}

	if(!get_bool_text(config_root, "autopoll", &m_config.autopoll))
	{
		logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element autopoll");
		return false;
	}

	if(!get_bool_text(config_root, "listen_only", &m_config.listen_only))
	{
		logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element listen_only");
		return false;
	}

	if(!get_bool_text(config_root, "auto_startup", &m_config.auto_startup))
	{
		logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element auto_startup");
		return false;
	}
	
	{
		char const * timesync_mode_str = nullptr;
		if(!get_str_text(config_root, "timesync_mode", &timesync_mode_str))
		{
			logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element timesync_mode");
			return false;
		}

		const char master_str[] = "master";
		const char slave_str[] = "slave";
		if(strncasecmp(timesync_mode_str, master_str, strlen(master_str)) == 0)
		{
			m_config.timesync_mode = CAN_USB_app_config::TIMESYNC_MODE::MASTER;
		}	
		else if(strncasecmp(timesync_mode_str, slave_str, strlen(slave_str)) == 0)
		{
			m_config.timesync_mode = CAN_USB_app_config::TIMESYNC_MODE::SLAVE;
		}
		else
		{
			logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not parse element timesync_mode");
			return false;
		}
	}

	{
		char const * slope_ctrl_str = nullptr;
		if(!get_str_text(config_root, "slope_ctrl", &slope_ctrl_str))
		{
			logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element slope_ctrl");
			return false;
		}

		if(strncasecmp(slope_ctrl_str, "SLOW", 4) == 0)
		{
			m_config.slope_ctrl = CAN_USB_app_config::SLOPE_CONTROL::SLOW;
		}	
		else if(strncasecmp(slope_ctrl_str, "FAST", 4) == 0)
		{
			m_config.slope_ctrl = CAN_USB_app_config::SLOPE_CONTROL::FAST;
		}
		else if(strncasecmp(slope_ctrl_str, "AUTO", 4) == 0)
		{
			m_config.slope_ctrl = CAN_USB_app_config::SLOPE_CONTROL::AUTO;
		}
		else
		{
			logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not parse element fast_slope");
			return false;
		}
	}

	{
		const tinyxml2::XMLElement* timestamp_element = config_root->FirstChildElement("timestamp");
		if(timestamp_element == nullptr)
		{
			logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element timestamp");
			return false;
		}

		if(!get_bool_text(timestamp_element, "enable", &m_config.timestamp_enable))
		{
			logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element timestamp/enable");
			return false;
		}
		
		if(!get_uint_text(timestamp_element, "prescaler", &m_config.timestamp_prescaler))
		{
			logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element timestamp/prescaler");
			return false;
		}

		if(!get_uint_text(timestamp_element, "period", &m_config.timestamp_period))
		{
			logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element timestamp/period");
			return false;
		}
	}

	{
		const tinyxml2::XMLElement* tx_delay_comp_element = config_root->FirstChildElement("tx_delay_comp");
		if(tx_delay_comp_element == nullptr)
		{
			logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element tx_delay_comp");
			return false;
		}

		if(!get_bool_text(tx_delay_comp_element, "enable", &m_config.tx_delay_comp_enable))
		{
			logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element tx_delay_comp/enable");
			return false;
		}

		if(!get_uint_text(tx_delay_comp_element, "offset", &m_config.tx_delay_comp_offset))
		{
			logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element tx_delay_comp/offset");
			return false;
		}

		if(!get_uint_text(tx_delay_comp_element, "filter_window", &m_config.tx_delay_comp_filter_window))
		{
			logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element tx_delay_comp/filter_window");
			return false;
		}
	}

	{
		if(!get_uint_text(config_root, "clock", &m_config.can_clock))
		{
			logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element clock");
			return false;
		}
	}

	{
		const tinyxml2::XMLElement* bitrate_element = config_root->FirstChildElement("bitrate");
		if(bitrate_element == nullptr)
		{
			logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element bitrate");
			return false;
		}

		if(!get_uint_text(bitrate_element, "nominal", &m_config.bitrate_nominal))
		{
			logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element bitrate/nominal");
			return false;
		}
		
		if(!get_uint_text(bitrate_element, "data", &m_config.bitrate_data))
		{
			logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element bitrate/data");
			return false;
		}
	}

	{
		const tinyxml2::XMLElement* protocol_element = config_root->FirstChildElement("protocol");
		if(protocol_element == nullptr)
		{
			logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element protocol");
			return false;
		}

		if(!get_bool_text(protocol_element, "ext_id", &m_config.protocol_ext_id))
		{
			logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element protocol/ext_id");
			return false;
		}
		
		if(!get_bool_text(protocol_element, "fd", &m_config.protocol_fd))
		{
			logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element protocol/fd");
			return false;
		}
		
		if(!get_bool_text(protocol_element, "brs", &m_config.protocol_brs))
		{
			logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element protocol/brs");
			return false;
		}

		if(!get_bool_text(protocol_element, "fd_iso", &m_config.protocol_fd_iso))
		{
			logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element protocol/fd_iso");
			return false;
		}
	}

	{
		const tinyxml2::XMLElement* filter_element = config_root->FirstChildElement("filter");
		if(filter_element == nullptr)
		{
			logger->log(LOG_LEVEL::WARN, "CAN_USB_app", "config.xml: could not find element filter, disabling packet filter");
			m_config.sja1000_filter.set_default();
		}
		else
		{
			char const * filter_type_str = nullptr;
			if(filter_element->QueryStringAttribute("type", &filter_type_str) != tinyxml2::XML_SUCCESS)
			{
				logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find attr filter::type");
				return false;
			}

			const char sja1000_str[] = "sja1000";
			if(strncasecmp(filter_type_str, sja1000_str, strlen(sja1000_str)) == 0)
			{
				logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: attr filter::type is not sja1000");
				return false;
			}

			if(!get_bool_text(filter_element, "enable", &m_config.sja1000_filter.enable))
			{
				logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element filter/enable");
				return false;
			}
			
			char const * mode_str = nullptr;
			if(!get_str_text(filter_element, "mode", &mode_str))
			{
				logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element filter/mode");
				return false;
			}

			const char dual_str[] = "DUAL";
			const char single_str[] = "SINGLE";
			if(strncasecmp(mode_str, dual_str, strlen(dual_str)) == 0)
			{
				m_config.sja1000_filter.mode = SJA1000_filter::FILTER_MODE::DUAL;
			}
			else if(strncasecmp(mode_str, single_str, strlen(single_str)) == 0)
			{
				m_config.sja1000_filter.mode = SJA1000_filter::FILTER_MODE::SINGLE;
			}
			else
			{
				logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not parse element filter/mode");
				return false;
			}

			unsigned temp = 0;
			if(!get_hex_text(filter_element, "accept_code", &temp))
			{
				logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element filter/accept_code");
				return false;
			}
			m_config.sja1000_filter.accept_code = temp;
			
			if(!get_hex_text(filter_element, "accept_mask", &temp))
			{
				logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element filter/accept_mask");
				return false;
			}
			m_config.sja1000_filter.accept_mask = temp;
		}
	}

	{
		const tinyxml2::XMLElement* debug_element = config_root->FirstChildElement("debug");
		if(debug_element == nullptr)
		{
			logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element debug");
			return false;
		}
	
		{
			char const * log_level_str = nullptr;
			if(!get_str_text(debug_element, "log_level", &log_level_str))
			{
				logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element debug/log_level");
				return false;
			}

			if(strncasecmp(log_level_str, "FATAL", 4) == 0)
			{
				m_config.log_level = freertos_util::logging::LOG_LEVEL::FATAL;
			}	
			else if(strncasecmp(log_level_str, "ERROR", 5) == 0)
			{
				m_config.log_level = freertos_util::logging::LOG_LEVEL::ERROR;
			}
			else if(strncasecmp(log_level_str, "WARN", 4) == 0)
			{
				m_config.log_level = freertos_util::logging::LOG_LEVEL::WARN;
			}
			else if(strncasecmp(log_level_str, "INFO", 4) == 0)
			{
				m_config.log_level = freertos_util::logging::LOG_LEVEL::INFO;
			}
			else if(strncasecmp(log_level_str, "DEBUG", 5) == 0)
			{
				m_config.log_level = freertos_util::logging::LOG_LEVEL::DEBUG;
			}
			else if(strncasecmp(log_level_str, "TRACE", 5) == 0)
			{
				m_config.log_level = freertos_util::logging::LOG_LEVEL::TRACE;
			}
			else
			{
				logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not parse element debug/log_level");
				return false;
			}
		}

		if(!get_uint_text(debug_element, "uart_baud", &m_config.uart_baud))
		{
			logger->log(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element debug/uart_baud");
			return false;
		}
	}

	return true;
}
