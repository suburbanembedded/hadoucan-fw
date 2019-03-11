#pragma once

#include "W25Q16JV.hpp"
#include "W25Q16JV_conf_region.hpp"

#include "../external/tinyxml2/tinyxml2.h"

#include <map>

class CAN_USB_app
{
public:
	CAN_USB_app();
	~CAN_USB_app();

	bool load_config();
	bool load_bitrate_table();

	class CAN_Config
	{
	public:
		CAN_Config()
		{
			set_defualt();
		}

		void set_defualt()
		{			
			autopoll = false;
			listen_only = false;
			timestamp_enable = false;
			timestamp_prescaler =  2000;
			timestamp_period = 50000;
			can_clock = 60000000;
			bitrate_nominal = 500000;
			bitrate_data = 4000000;
			protocol_ext_id = true;
			protocol_fd = true;
			protocol_brs = false;
			filter_accept_enable = false;
			filter_accept_code = 0x00000000;
			filter_accept_mask = 0x00000000;
		}

		bool autopoll;
		bool listen_only;

		bool timestamp_enable;
		int timestamp_prescaler;
		int timestamp_period;

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

	struct Bitrate_Table_Entry
	{
		int rate;
		int pre;
		int tseg1;
		int tseg2;
		int sjw;
	};

	struct Bitrate_Table
	{
		//key on bitrate
		std::map<int, Bitrate_Table_Entry> m_nominal_table;
		std::map<int, Bitrate_Table_Entry> m_data_table;
	};

	struct Bitrate_Table_Set
	{
		//key on clock speed
		std::map<int, Bitrate_Table> m_tables;
	};

	bool write_default_config();
	bool write_default_bitrate_table();

	W25Q16JV& get_flash()
	{
		return m_qspi; 
	}
	W25Q16JV_conf_region& get_fs()
	{
		return m_fs; 
	}

	const CAN_Config& get_config() const
	{
		return m_config;
	}
	const Bitrate_Table_Set& get_bitrate_tables() const
	{
		return m_bitrate_tables;
	}

protected:

	CAN_Config m_config;
	Bitrate_Table_Set m_bitrate_tables;

	bool write_xml_file(const char* name, const tinyxml2::XMLDocument& xml);
	bool load_xml_file(const char* name, tinyxml2::XMLDocument* const out_xml);

	static bool get_bool_text(const tinyxml2::XMLElement* root, const char* child, bool* const out_val);

	static bool get_int_text(const tinyxml2::XMLElement* root, const char* child, int* const out_val);

	static bool get_hex_text(const tinyxml2::XMLElement* root, const char* child, unsigned* const out_val);

	static bool get_str_text(const tinyxml2::XMLElement* root, const char* child, char const * * const out_val);

	static bool parse_config(const tinyxml2::XMLDocument& config_doc, CAN_Config* const out_config);

	static bool parse_bitrate_tables(const tinyxml2::XMLDocument& bitrate_tables_doc, Bitrate_Table_Set* const out_table_set);

	W25Q16JV m_qspi;
	W25Q16JV_conf_region m_fs;
};