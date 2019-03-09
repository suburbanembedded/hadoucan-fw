#pragma once

#include "spiffs_int_w25q16jv.hpp"

class W25Q16JV_conf_region : public spiffs_int_w25q16jv
{
public:

	size_t get_start_bytes() override
	{
		//bottom 24*64k is application
		//top     8*64k is config
		return 24*64*1024;
	}
	size_t get_len_bytes() override
	{
		// total size in bytes 2*1024*1024;
		// total block64 32*64k;

		//assign 25% or 512kB to this region

		return 8*64*1024;
	}
};