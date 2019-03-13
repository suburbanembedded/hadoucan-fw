#pragma once

#include "tinyxml2_helper.hpp"
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

	struct Bitrate_Table
	{
		//key on bitrate
		std::map<int, Bitrate_Table_Entry> m_nominal_table;
		std::map<int, Bitrate_Table_Entry> m_data_table;
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

protected:
	Bitrate_Table_Set m_bitrate_tables;
};