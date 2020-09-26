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

	enum class HOST_PROTOCOL
	{
		LAWICEL_CAN232 = 0
	};

	struct Simple_filter
	{
		enum class FRAME_TYPE
		{
			STD,
			EXT
		};

		bool enable;
		FRAME_TYPE mode;
		unsigned accept_code;
		unsigned accept_mask;
	};

	// Lawicel AB CAN232 compat mode
	// Emulate SJA1000 filter
	struct SJA1000_filter
	{
		SJA1000_filter()
		{
			set_default();
		}

		enum class FILTER_MODE
		{
			DUAL   = 0,
			SINGLE = 1
		};

		void set_default()
		{
			enable      = false;
			mode        = FILTER_MODE::DUAL;
			accept_code = 0x00000000;
			accept_mask = 0xFFFFFFFF;
		}

		bool enable;

		FILTER_MODE mode;

		// ACR0[7..0] | ACR1[7..0] | ACR2[7..0] | ACR3[7..0]
		unsigned accept_code;

		// AMR0[7..0] | AMR1[7..0] | AMR2[7..0] | AMR3[7..0]
		unsigned accept_mask;

		//In single frame mode
		//For Std, ACR0[7..0], ACR1[7..0], ACR2[7..0], ACR3[7..0] maps to can[10..0, RTR, X, X, X, X, data[7..0], data[15..8]
		//For Ext, ACR0[7..0], ACR1[7..0], ACR2[7..0], ACR3[7..0] maps to can[29..0, RTR, X, X]

		//In dual frame mode
		//For Std,
		//        ACR0[7..0], ACR1[7..4] maps to can[10..0, RTR] <-- either filter can accept
		//        ACR1[3..0], ACR3[3..0] maps to data[7..0]
		//        ACR2[3..0], ACR3[7..4] maps to can[10..0, RTR] <-- either filter can accept
		//For Ext,
		//        ACR0[7..0], ACR1[7..0] maps to can[28..13] <-- either filter can accept
		//        ACR2[3..0], ACR3[7..0] maps to can[28..13] <-- either filter can accept
	};

	struct Config_Set
	{
		unsigned config_version;

		HOST_PROTOCOL host_protocol;

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

		SJA1000_filter sja1000_filter;

		freertos_util::logging::LOG_LEVEL log_level;
		unsigned uart_baud;
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
