#pragma once

#include "../external/tinyxml2/tinyxml2.h"

bool get_bool_text(const tinyxml2::XMLElement* root, const char* child, bool* const out_val);
bool get_int_text(const tinyxml2::XMLElement* root, const char* child, int* const out_val);
bool get_hex_text(const tinyxml2::XMLElement* root, const char* child, unsigned* const out_val);
bool get_str_text(const tinyxml2::XMLElement* root, const char* child, char const * * const out_val);
