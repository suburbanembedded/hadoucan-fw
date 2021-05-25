#pragma once

#include "freertos_cpp_util/logging/Logger_types.hpp"

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

		bool auto_startup;

		TIMESYNC_MODE timesync_mode;

		SLOPE_CONTROL slope_ctrl;

		bool timestamp_enable;
		unsigned timestamp_prescaler;
		unsigned timestamp_period;

		bool tx_delay_comp_enable;
		unsigned tx_delay_comp_offset;
		unsigned tx_delay_comp_filter_window;

		unsigned can_clock;

		unsigned bitrate_nominal;
		unsigned bitrate_data;

		bool protocol_ext_id;
		bool protocol_fd;
		bool protocol_brs;
		bool protocol_fd_iso;

		bool filter_accept_enable;
		unsigned filter_accept_code;
		unsigned filter_accept_mask;

		freertos_util::logging::LOG_LEVEL log_level;
		unsigned uart_baud;

		unsigned usb_tx_delay;
		unsigned can_rx_poll_interval;
		unsigned can_rx_isr_watermark;
	};

	static Config_Set get_defualt()
	{
		CAN_USB_app_config config;
		config.set_defualt();

		return config.m_config;
	}

	void set_defualt();

	bool to_xml(tinyxml2::XMLDocument* const config_doc) const;
	bool from_xml(const tinyxml2::XMLDocument& config_doc);

	Config_Set& get_config()
	{
		return m_config;
	}

	const Config_Set& get_config() const
	{
		return m_config;
	}

protected:
	Config_Set m_config;
};
