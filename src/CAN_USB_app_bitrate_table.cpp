#include "CAN_USB_app_bitrate_table.hpp"

#include "uart1_printf.hpp"

void CAN_USB_app_bitrate_table::set_defualt()
{
	Bitrate_Table_Entry entry;
	int clock = 0;
	
	/// 24MHz
	clock = 24000000;
	entry.rate  = 5000;
	entry.pre   = 192;
	entry.tseg1 = 16;
	entry.tseg2 = 8;
	entry.sjw   = 2;
	m_bitrate_tables[clock].m_nominal_table[entry.rate] = entry;

	entry.rate  = 10000;
	entry.pre   = 120;
	entry.tseg1 = 16;
	entry.tseg2 = 3;
	entry.sjw   = 2;
	m_bitrate_tables[clock].m_nominal_table[entry.rate] = entry;

	entry.rate  = 20000;
	entry.pre   = 60;
	entry.tseg1 = 16;
	entry.tseg2 = 3;
	entry.sjw   = 2;
	m_bitrate_tables[clock].m_nominal_table[entry.rate] = entry;

	entry.rate  = 50000;
	entry.pre   = 24;
	entry.tseg1 = 16;
	entry.tseg2 = 3;
	entry.sjw   = 2;
	m_bitrate_tables[clock].m_nominal_table[entry.rate] = entry;

	entry.rate  = 100000;
	entry.pre   = 12;
	entry.tseg1 = 16;
	entry.tseg2 = 3;
	entry.sjw   = 2;
	m_bitrate_tables[clock].m_nominal_table[entry.rate] = entry;

	entry.rate  = 125000;
	entry.pre   = 12;
	entry.tseg1 = 13;
	entry.tseg2 = 3;
	entry.sjw   = 1;
	m_bitrate_tables[clock].m_nominal_table[entry.rate] = entry;

	entry.rate  = 250000;
	entry.pre   = 6;
	entry.tseg1 = 12;
	entry.tseg2 = 2;
	entry.sjw   = 1;
	m_bitrate_tables[clock].m_nominal_table[entry.rate] = entry;

	entry.rate  = 500000;
	entry.pre   = 3;
	entry.tseg1 = 13;
	entry.tseg2 = 2;
	entry.sjw   = 1;
	m_bitrate_tables[clock].m_nominal_table[entry.rate] = entry;

	entry.rate  = 800000;
	entry.pre   = 3;
	entry.tseg1 = 7;
	entry.tseg2 = 2;
	entry.sjw   = 1;
	m_bitrate_tables[clock].m_nominal_table[entry.rate] = entry;

	entry.rate  = 1000000;
	entry.pre   = 3;
	entry.tseg1 = 5;
	entry.tseg2 = 2;
	entry.sjw   = 1;
	m_bitrate_tables[clock].m_nominal_table[entry.rate] = entry;

	entry.rate  = 2000000;
	entry.pre   = 2;
	entry.tseg1 = 4;
	entry.tseg2 = 1;
	entry.sjw   = 1;
	m_bitrate_tables[clock].m_data_table[entry.rate] = entry;

	entry.rate  = 6000000;
	entry.pre   = 1;
	entry.tseg1 = 1;
	entry.tseg2 = 2;
	entry.sjw   = 1;
	m_bitrate_tables[clock].m_data_table[entry.rate] = entry;

	/// 60MHz
	clock = 60000000;
	entry.rate  = 250000;
	entry.pre   = 24;
	entry.tseg1 = 7;
	entry.tseg2 = 2;
	entry.sjw   = 1;
	m_bitrate_tables[clock].m_nominal_table[entry.rate] = entry;

	entry.rate  = 500000;
	entry.pre   = 12;
	entry.tseg1 = 7;
	entry.tseg2 = 2;
	entry.sjw   = 1;
	m_bitrate_tables[clock].m_nominal_table[entry.rate] = entry;

	entry.rate  = 1000000;
	entry.pre   = 5;
	entry.tseg1 = 8;
	entry.tseg2 = 3;
	entry.sjw   = 1;
	m_bitrate_tables[clock].m_nominal_table[entry.rate] = entry;

	entry.rate  = 2000000;
	entry.pre   = 3;
	entry.tseg1 = 7;
	entry.tseg2 = 2;
	entry.sjw   = 1;
	m_bitrate_tables[clock].m_data_table[entry.rate] = entry;

	entry.rate  = 4000000;
	entry.pre   = 3;
	entry.tseg1 = 3;
	entry.tseg2 = 1;
	entry.sjw   = 1;
	m_bitrate_tables[clock].m_data_table[entry.rate] = entry;

	entry.rate  = 6000000;
	entry.pre   = 2;
	entry.tseg1 = 3;
	entry.tseg2 = 1;
	entry.sjw   = 1;
	m_bitrate_tables[clock].m_data_table[entry.rate] = entry;

	entry.rate  = 10000000;
	entry.pre   = 1;
	entry.tseg1 = 3;
	entry.tseg2 = 2;
	entry.sjw   = 1;
	m_bitrate_tables[clock].m_data_table[entry.rate] = entry;

	entry.rate  = 12000000;
	entry.pre   = 1;
	entry.tseg1 = 2;
	entry.tseg2 = 2;
	entry.sjw   = 1;
	m_bitrate_tables[clock].m_data_table[entry.rate] = entry;

	/// 80MHz
	clock = 80000000;
	entry.rate  = 250000;
	entry.pre   = 20;
	entry.tseg1 = 12;
	entry.tseg2 = 3;
	entry.sjw   = 1;
	m_bitrate_tables[clock].m_nominal_table[entry.rate] = entry;

	entry.rate  = 500000;
	entry.pre   = 10;
	entry.tseg1 = 12;
	entry.tseg2 = 3;
	entry.sjw   = 1;
	m_bitrate_tables[clock].m_nominal_table[entry.rate] = entry;

	entry.rate  = 1000000;
	entry.pre   = 10;
	entry.tseg1 = 5;
	entry.tseg2 = 2;
	entry.sjw   = 1;
	m_bitrate_tables[clock].m_nominal_table[entry.rate] = entry;

	entry.rate  = 2000000;
	entry.pre   = 4;
	entry.tseg1 = 7;
	entry.tseg2 = 2;
	entry.sjw   = 1;
	m_bitrate_tables[clock].m_data_table[entry.rate] = entry;

	entry.rate  = 4000000;
	entry.pre   = 2;
	entry.tseg1 = 7;
	entry.tseg2 = 2;
	entry.sjw   = 1;
	m_bitrate_tables[clock].m_data_table[entry.rate] = entry;

	entry.rate  = 8000000;
	entry.pre   = 4;
	entry.tseg1 = 7;
	entry.tseg2 = 2;
	entry.sjw   = 1;
	m_bitrate_tables[clock].m_data_table[entry.rate] = entry;

	entry.rate  = 10000000;
	entry.pre   = 1;
	entry.tseg1 = 5;
	entry.tseg2 = 2;
	entry.sjw   = 1;
	m_bitrate_tables[clock].m_data_table[entry.rate] = entry;
}

bool CAN_USB_app_bitrate_table::to_xml(tinyxml2::XMLDocument* const table_doc) const
{
	table_doc->Clear();

	{
		tinyxml2::XMLDeclaration* const decl = table_doc->NewDeclaration("xml version=\"1.0\" standalone=\"yes\" encoding=\"UTF-8\"");
		table_doc->InsertFirstChild(decl);
	}

	tinyxml2::XMLElement* const table_doc_root = table_doc->NewElement("bitrate_tables");
	table_doc->InsertEndChild(table_doc_root);

	for(const auto& table : m_bitrate_tables)
	{
		tinyxml2::XMLElement* table_element = table_doc->NewElement("table");
		table_element->SetAttribute("clock", table.first);
		table_doc_root->InsertEndChild(table_element);

		for(const auto& nominal : table.second.m_nominal_table)
		{
			tinyxml2::XMLElement* entry = table_doc->NewElement("entry");
			entry->SetAttribute("type",  "nominal");
			entry->SetAttribute("rate",  nominal.second.rate);
			entry->SetAttribute("pre",   nominal.second.pre);
			entry->SetAttribute("tseg1", nominal.second.tseg1);
			entry->SetAttribute("tseg2", nominal.second.tseg2);
			entry->SetAttribute("sjw",   nominal.second.sjw);
			table_element->InsertEndChild(entry);
		}

		for(const auto& data : table.second.m_data_table)
		{
			tinyxml2::XMLElement* entry = table_doc->NewElement("entry");
			entry->SetAttribute("type",  "data");
			entry->SetAttribute("rate",  data.second.rate);
			entry->SetAttribute("pre",   data.second.pre);
			entry->SetAttribute("tseg1", data.second.tseg1);
			entry->SetAttribute("tseg2", data.second.tseg2);
			entry->SetAttribute("sjw",   data.second.sjw);
			table_element->InsertEndChild(entry);
		}
	}

	return true;
}
bool CAN_USB_app_bitrate_table::from_xml(const tinyxml2::XMLDocument& table_doc)
{
	m_bitrate_tables.clear();

	const tinyxml2::XMLElement* const bitrate_tables_root = table_doc.FirstChildElement("bitrate_tables");
	if(bitrate_tables_root == nullptr)
	{
		uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "table.xml: could not find element bitrate_tables");
		return false;
	}

	tinyxml2::XMLElement const * table_element = bitrate_tables_root->FirstChildElement("table");
	if(table_element == nullptr)
	{
		uart1_log<128>(LOG_LEVEL::ERROR, "CAN_USB_app", "No bitrate table found");
		return false;
	}

	do
	{
		int clock = 0;
		if(table_element->QueryIntAttribute("clock", &clock) != tinyxml2::XML_SUCCESS)
		{
			uart1_log<128>(LOG_LEVEL::WARN, "CAN_USB_app", "Could not get attribute named clock for table, skipping table");
			continue;
		}

		tinyxml2::XMLElement const * entry_element = table_element->FirstChildElement("entry");
		if(entry_element == nullptr)
		{
			uart1_log<128>(LOG_LEVEL::WARN, "CAN_USB_app", "Empty bitrate_table for clock %d, skipping table", clock);
			continue;
		}

		//the table for this clock setting
		CAN_USB_app_bitrate_table::Bitrate_Table& bitrate_table = m_bitrate_tables[clock];

		do
		{
			CAN_USB_app_bitrate_table::Bitrate_Table_Entry entry;

			char const* type = nullptr;
			if(entry_element->QueryStringAttribute("type", &type) != tinyxml2::XML_SUCCESS)
			{
				return false;
			}

			entry.rate = 0;
			if(entry_element->QueryIntAttribute("rate", &entry.rate) != tinyxml2::XML_SUCCESS)
			{
				return false;
			}

			entry.pre = 0;
			if(entry_element->QueryIntAttribute("pre", &entry.pre) != tinyxml2::XML_SUCCESS)
			{
				return false;
			}

			entry.tseg1 = 0;
			if(entry_element->QueryIntAttribute("tseg1", &entry.tseg1) != tinyxml2::XML_SUCCESS)
			{
				return false;
			}

			entry.tseg2 = 0;
			if(entry_element->QueryIntAttribute("tseg2", &entry.tseg2) != tinyxml2::XML_SUCCESS)
			{
				return false;
			}

			entry.sjw = 0;
			if(entry_element->QueryIntAttribute("sjw", &entry.sjw) != tinyxml2::XML_SUCCESS)
			{
				return false;
			}

			const std::string type_str(type);
			const std::string nominal_str("nominal");
			const std::string data_str("data");
			if(type_str == nominal_str)
			{
				bitrate_table.m_nominal_table[entry.rate] = entry;
			}
			else if(type_str == data_str)
			{
				bitrate_table.m_data_table[entry.rate] = entry;
			}
			else
			{
				uart1_log<128>(LOG_LEVEL::WARN, "CAN_USB_app", "Dropping bitrate_table entry of unknown type %s", type);
				continue;
			}

			entry_element = entry_element->NextSiblingElement("entry");

		} while(entry_element != nullptr);

		table_element = table_element->NextSiblingElement("table");

	} while(table_element != nullptr);

	return true;
}
