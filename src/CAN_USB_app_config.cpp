#include "CAN_USB_app_config.hpp"

#include "uart1_printf.hpp"

bool CAN_USB_app_config::to_xml(tinyxml2::XMLDocument* const config_doc) const
{
	config_doc->Clear();

	{
		tinyxml2::XMLDeclaration* decl = config_doc->NewDeclaration("xml version=\"1.0\" standalone=\"yes\" encoding=\"UTF-8\"");
		config_doc->InsertFirstChild(decl);
	}

	tinyxml2::XMLElement* config_doc_root = config_doc->NewElement("config");
	config_doc_root->SetAttribute("version", m_config.config_version);
	config_doc->InsertEndChild(config_doc_root);


	//General Config Settings
	tinyxml2::XMLElement* node = config_doc->NewElement("autopoll");
	node->SetText(m_config.autopoll);
	config_doc_root->InsertEndChild(node);

	node = config_doc->NewElement("listen_only");
	node->SetText(m_config.listen_only);
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
	}

	{
		tinyxml2::XMLElement* filter = config_doc->NewElement("filter");
		config_doc_root->InsertEndChild(filter);

		node = config_doc->NewElement("accept_code");
		node->SetText("00000000");
		filter->InsertEndChild(node);

		node = config_doc->NewElement("accept_mask");
		node->SetText("FFFFFFFF");
		filter->InsertEndChild(node);
	}

	{
		tinyxml2::XMLElement* debug = config_doc->NewElement("debug");
		config_doc_root->InsertEndChild(debug);

		tinyxml2::XMLComment* comment = config_doc->NewComment("log_level may be TRACE, DEBUG, INFO, WARN, ERROR, FATAL");
		debug->InsertEndChild(comment);

		node = config_doc->NewElement("log_level");
		node->SetText("DEBUG");
		debug->InsertEndChild(node);

		node = config_doc->NewElement("baud");
		node->SetText(115200U);
		debug->InsertEndChild(node);
	}

	return true;
}

bool CAN_USB_app_config::from_xml(const tinyxml2::XMLDocument& config_doc)
{
	const tinyxml2::XMLElement* const config_root = config_doc.FirstChildElement("config");
	if(config_root == nullptr)
	{
		uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element config");
		return false;
	}

	if(config_root->QueryUnsignedAttribute("version", &m_config.config_version) != tinyxml2::XML_SUCCESS)
	{
		uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find attr version");
		return false;
	}

	if(!get_bool_text(config_root, "autopoll", &m_config.autopoll))
	{
		uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element autopoll");
		return false;
	}

	if(!get_bool_text(config_root, "listen_only", &m_config.listen_only))
	{
		uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element listen_only");
		return false;
	}
	
	{
		char const * timesync_mode_str = nullptr;
		if(!get_str_text(config_root, "timesync_mode", &timesync_mode_str))
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element timesync_mode");
			return false;
		}

		if(strncasecmp(timesync_mode_str, "master", 4) == 0)
		{
			m_config.timesync_mode = CAN_USB_app_config::TIMESYNC_MODE::MASTER;
		}	
		else if(strncasecmp(timesync_mode_str, "slave", 4) == 0)
		{
			m_config.timesync_mode = CAN_USB_app_config::TIMESYNC_MODE::SLAVE;
		}
		else
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not parse element timesync_mode");
			return false;
		}
	}

	{
		char const * slope_ctrl_str = nullptr;
		if(!get_str_text(config_root, "slope_ctrl", &slope_ctrl_str))
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element slope_ctrl");
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
			uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not parse element fast_slope");
			return false;
		}
	}

	{
		const tinyxml2::XMLElement* timestamp_element = config_root->FirstChildElement("timestamp");
		if(timestamp_element == nullptr)
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element timestamp");
			return false;
		}

		if(!get_bool_text(timestamp_element, "enable", &m_config.timestamp_enable))
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element timestamp/enable");
			return false;
		}
		
		if(!get_int_text(timestamp_element, "prescaler", &m_config.timestamp_prescaler))
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element timestamp/prescaler");
			return false;
		}

		if(!get_int_text(timestamp_element, "period", &m_config.timestamp_period))
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element timestamp/period");
			return false;
		}
	}

	{
		const tinyxml2::XMLElement* tx_delay_comp_element = config_root->FirstChildElement("tx_delay_comp");
		if(tx_delay_comp_element == nullptr)
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element tx_delay_comp");
			return false;
		}

		if(!get_bool_text(tx_delay_comp_element, "enable", &m_config.tx_delay_comp_enable))
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element tx_delay_comp/enable");
			return false;
		}

		if(!get_int_text(tx_delay_comp_element, "offset", &m_config.tx_delay_comp_offset))
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element tx_delay_comp/offset");
			return false;
		}

		if(!get_int_text(tx_delay_comp_element, "filter_window", &m_config.tx_delay_comp_filter_window))
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element tx_delay_comp/filter_window");
			return false;
		}
	}

	{
		if(!get_int_text(config_root, "clock", &m_config.can_clock))
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element clock");
			return false;
		}
	}

	{
		const tinyxml2::XMLElement* bitrate_element = config_root->FirstChildElement("bitrate");
		if(bitrate_element == nullptr)
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element bitrate");
			return false;
		}

		if(!get_int_text(bitrate_element, "nominal", &m_config.bitrate_nominal))
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element bitrate/nominal");
			return false;
		}
		
		if(!get_int_text(bitrate_element, "data", &m_config.bitrate_data))
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element bitrate/data");
			return false;
		}
	}

	{
		const tinyxml2::XMLElement* protocol_element = config_root->FirstChildElement("protocol");
		if(protocol_element == nullptr)
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element protocol");
			return false;
		}

		if(!get_bool_text(protocol_element, "ext_id", &m_config.protocol_ext_id))
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element protocol/ext_id");
			return false;
		}
		
		if(!get_bool_text(protocol_element, "fd", &m_config.protocol_fd))
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element protocol/fd");
			return false;
		}
		
		if(!get_bool_text(protocol_element, "brs", &m_config.protocol_brs))
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element protocol/brs");
			return false;
		}
	}

	{
		const tinyxml2::XMLElement* filter_element = config_root->FirstChildElement("filter");
		if(filter_element == nullptr)
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element filter");
			return false;
		}
		
		if(!get_hex_text(filter_element, "accept_code", &m_config.filter_accept_code))
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element filter/accept_code");
			return false;
		}
		
		if(!get_hex_text(filter_element, "accept_mask", &m_config.filter_accept_mask))
		{
			uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "config.xml: could not find element filter/accept_mask");
			return false;
		}
	}

	return true;
}
