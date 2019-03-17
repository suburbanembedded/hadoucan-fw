#pragma once

#include "tinyxml2_util/tinyxml2_helper.hpp"
#include "../external/tinyxml2/tinyxml2.h"

#include <map>

class CAN_USB_app_bitrate_table
{
public:
	CAN_USB_app_bitrate_table() = default;
	~CAN_USB_app_bitrate_table() = default;

	struct Bitrate_Table_Entry
	{
		int rate;
		int pre;
		int tseg1;
		int tseg2;
		int sjw;
	};

	//key on clock speed
	typedef std::map<int, Bitrate_Table_Entry> Bitrate_Table_map;

	struct Bitrate_Table
	{
		//key on bitrate
		Bitrate_Table_map m_nominal_table;
		Bitrate_Table_map m_data_table;
	};

	//key on clock speed
	typedef std::map<int, Bitrate_Table> Bitrate_Table_Set;

	void set_defualt();

	static bool to_xml(const Bitrate_Table_Entry& entry, tinyxml2::XMLElement* const elem);

	bool to_xml(tinyxml2::XMLDocument* const table_doc) const;
	bool from_xml(const tinyxml2::XMLDocument& table_doc);

	const Bitrate_Table_Set& get_table() const
	{
		return m_bitrate_tables;
	}

	bool get_nominal_entry(int can_clock, int nominal_bitrate, Bitrate_Table_Entry* const entry) const
	{
		CAN_USB_app_bitrate_table::Bitrate_Table_Set::const_iterator table = m_bitrate_tables.find(can_clock);
		if(table == m_bitrate_tables.end())
		{
			return false;
		}

		CAN_USB_app_bitrate_table::Bitrate_Table_map::const_iterator nominal_entry_it = table->second.m_nominal_table.find(nominal_bitrate);
		if(nominal_entry_it == table->second.m_nominal_table.end())
		{
			return false;
		}
		
		*entry = nominal_entry_it->second;
		return true;
	}

	bool get_data_entry(int can_clock, int data_bitrate, Bitrate_Table_Entry* const entry) const
	{
		CAN_USB_app_bitrate_table::Bitrate_Table_Set::const_iterator table = m_bitrate_tables.find(can_clock);
		if(table == m_bitrate_tables.end())
		{
			return false;
		}

		CAN_USB_app_bitrate_table::Bitrate_Table_map::const_iterator data_entry_it = table->second.m_data_table.find(data_bitrate);
		if(data_entry_it == table->second.m_data_table.end())
		{
			return false;
		}
		
		*entry = data_entry_it->second;
		return true;
	}

protected:
	Bitrate_Table_Set m_bitrate_tables;
};