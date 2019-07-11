#pragma once

#include "CAN_USB_app_bitrate_table.hpp"
#include "CAN_USB_app_config.hpp"

#include "W25Q16JV.hpp"
#include "W25Q16JV_conf_region.hpp"

#include "STM32_fdcan_tx.hpp"

#include "freertos_cpp_util/Mutex_static_recursive.hpp"

#include "../external/tinyxml2/tinyxml2.h"

#include <mutex>

class CAN_USB_app
{
public:
	CAN_USB_app();
	~CAN_USB_app();

	bool load_config();
	bool load_bitrate_table();

	bool write_config(const CAN_USB_app_config& config);
	bool write_default_config();

	bool write_bitrate_table(const CAN_USB_app_bitrate_table& table);
	bool write_default_bitrate_table();

	Mutex_static_recursive& get_mutex()
	{
		return m_mutex;
	}

	W25Q16JV& get_flash()
	{
		return m_qspi; 
	}
	W25Q16JV_conf_region& get_fs()
	{
		return m_fs; 
	}

	void get_config(CAN_USB_app_config* const out_config) const
	{
		std::lock_guard<Mutex_static_recursive> lock(m_mutex);
		*out_config = m_config;
	}
	void get_config(CAN_USB_app_config::Config_Set* const out_config) const
	{
		std::lock_guard<Mutex_static_recursive> lock(m_mutex);
		*out_config = m_config.get_config();
	}
	void get_bitrate_tables(CAN_USB_app_bitrate_table* const out_table) const
	{
		std::lock_guard<Mutex_static_recursive> lock(m_mutex);
		*out_table = m_bitrate_tables;
	}

	STM32_fdcan_tx& get_can_tx()
	{
		return can_tx;
	}

	static void get_unique_id(std::array<uint32_t, 3>* id);
	static void get_unique_id_str(std::array<char, 25>* id_str);

protected:

	CAN_USB_app_config m_config;
	CAN_USB_app_bitrate_table m_bitrate_tables;

	static bool write_xml_file(spiffs* const fs, const char* name, const tinyxml2::XMLDocument& xml);
	static bool load_xml_file(spiffs* const fs, const char* name, tinyxml2::XMLDocument* const out_xml);

	W25Q16JV m_qspi;
	W25Q16JV_conf_region m_fs;

	STM32_fdcan_tx can_tx;

	mutable Mutex_static_recursive m_mutex;
};