#pragma once

#include "tinyxml2_util/tinyxml2_helper.hpp"
#include "../external/tinyxml2/tinyxml2.h"

class CAN_USB_app_config
{
public:
	CAN_USB_app_config()
	{
		set_defualt();
	}

	enum class SLOPE_CONTROL
	{
		SLOW,
		FAST,
		AUTO
	};

	enum class TIMESYNC_MODE
	{
		MASTER,
		SLAVE
	};

	struct Config_Set
	{
		unsigned config_version;

		bool autopoll;
		bool listen_only;

		TIMESYNC_MODE timesync_mode;

		SLOPE_CONTROL slope_ctrl;

		bool timestamp_enable;
		int timestamp_prescaler;
		int timestamp_period;

		bool tx_delay_comp_enable;
		int tx_delay_comp_offset;
		int tx_delay_comp_filter_window;

		int can_clock;

		int bitrate_nominal;
		int bitrate_data;

		bool protocol_ext_id;
		bool protocol_fd;
		bool protocol_brs;

		bool filter_accept_enable;
		unsigned filter_accept_code;
		unsigned filter_accept_mask;
	};

	static Config_Set get_defualt()
	{
		CAN_USB_app_config config;
		config.set_defualt();

		return config.m_config;
	}

	void set_defualt()
	{	
		m_config.config_version = 0;

		m_config.autopoll = false;
		m_config.listen_only = false;

		m_config.timesync_mode = TIMESYNC_MODE::SLAVE;

		m_config.slope_ctrl = SLOPE_CONTROL::AUTO;

		m_config.timestamp_enable = false;
		m_config.timestamp_prescaler =  2000;
		m_config.timestamp_period = 50000;

		m_config.tx_delay_comp_enable = false;
		m_config.tx_delay_comp_offset = 5;
		m_config.tx_delay_comp_filter_window = 0;

		m_config.can_clock = 24000000;
		// m_config.can_clock = 60000000;
		m_config.bitrate_nominal = 500000;
		m_config.bitrate_data = 2000000;

		m_config.protocol_ext_id = true;
		m_config.protocol_fd = true;
		m_config.protocol_brs = false;

		m_config.filter_accept_enable = false;
		m_config.filter_accept_code = 0x00000000;
		m_config.filter_accept_mask = 0x00000000;
	}

	bool to_xml(tinyxml2::XMLDocument* const config_doc) const;
	bool from_xml(const tinyxml2::XMLDocument& config_doc);

	const Config_Set& get_config() const
	{
		return m_config;
	}

protected:
	Config_Set m_config;
};
