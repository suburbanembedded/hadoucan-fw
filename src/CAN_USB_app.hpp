#pragma once

#include "CAN_USB_app_bitrate_table.hpp"
#include "CAN_USB_app_config.hpp"

#include "W25Q16JV.hpp"
#include "W25Q16JV_conf_region.hpp"

#include "../external/tinyxml2/tinyxml2.h"

class CAN_USB_app
{
public:
	CAN_USB_app();
	~CAN_USB_app();

	bool load_config();
	bool load_bitrate_table();

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

	const CAN_USB_app_config::Config_Set& get_config() const
	{
		return m_config.get_config();
	}
	const CAN_USB_app_bitrate_table& get_bitrate_tables() const
	{
		return m_bitrate_tables;
	}

protected:

	CAN_USB_app_config m_config;
	CAN_USB_app_bitrate_table m_bitrate_tables;

	bool write_xml_file(const char* name, const tinyxml2::XMLDocument& xml);
	bool load_xml_file(const char* name, tinyxml2::XMLDocument* const out_xml);

	W25Q16JV m_qspi;
	W25Q16JV_conf_region m_fs;
};