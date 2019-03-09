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

	bool init_fs();

	bool load_config();
	bool load_bitrate_table();

	struct CAN_CONFIG
	{

	};

	struct Bitrate_Table_entry
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
		std::map<int, Bitrate_Table_entry> m_nominal_table;
		std::map<int, Bitrate_Table_entry> m_data_table;
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

protected:

	bool write_xml_file(const char* name, const tinyxml2::XMLDocument& xml);
	bool load_xml_file(const char* name, tinyxml2::XMLDocument* const out_xml);

	static bool get_bool_text(const tinyxml2::XMLElement* root, const char* child, bool* const out_val);

	static bool get_int_text(const tinyxml2::XMLElement* root, const char* child, int* const out_val);

	static bool get_hex_text(const tinyxml2::XMLElement* root, const char* child, unsigned* const out_val);

	static bool get_str_text(const tinyxml2::XMLElement* root, const char* child, char const * * const out_val);

	static bool parse_config(const tinyxml2::XMLDocument& config_doc, CAN_CONFIG* const out_config);

	static bool parse_bitrate_tables(const tinyxml2::XMLDocument& bitrate_tables_doc, Bitrate_Table_Set* const out_table_set);

	W25Q16JV m_qspi;
	W25Q16JV_conf_region m_fs;
};