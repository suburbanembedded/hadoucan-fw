#include "tinyxml2_helper.hpp"

bool get_bool_text(const tinyxml2::XMLElement* root, const char* child, bool* const out_val)
{
	if(root == nullptr)
	{
		return false;
	}

	const tinyxml2::XMLElement* node = root->FirstChildElement(child);
	if(node == nullptr)
	{
		return false;
	}
	if(node->QueryBoolText(out_val) != tinyxml2::XML_SUCCESS)
	{
		return false;
	}

	return true;
}

bool get_int_text(const tinyxml2::XMLElement* root, const char* child, int* const out_val)
{
	if(root == nullptr)
	{
		return false;
	}

	const tinyxml2::XMLElement* node = root->FirstChildElement(child);
	if(node == nullptr)
	{
		return false;
	}
	if(node->QueryIntText(out_val) != tinyxml2::XML_SUCCESS)
	{
		return false;
	}

	return true;
}

bool get_hex_text(const tinyxml2::XMLElement* root, const char* child, unsigned* const out_val)
{
	if(root == nullptr)
	{
		return false;
	}

	const tinyxml2::XMLElement* node = root->FirstChildElement(child);
	if(node == nullptr)
	{
		return false;
	}
	
	const char* str = node->GetText();
	if(str == nullptr)
	{
		return false;
	}

	if(sscanf(str, "%x", out_val) != 1)
	{
		return false;
	}

	return true;
}

bool get_str_text(const tinyxml2::XMLElement* root, const char* child, char const * * const out_val)
{
	if(root == nullptr)
	{
		return false;
	}

	const tinyxml2::XMLElement* node = root->FirstChildElement(child);
	if(node == nullptr)
	{
		return false;
	}
	
	const char* str = node->GetText();
	if(str == nullptr)
	{
		return false;
	}

	*out_val = str;

	return true;
}