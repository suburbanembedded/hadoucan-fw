#pragma once

#include "freertos_cpp_util/Task_static.hpp"

class Test_USB_Core_task : public Task_static<2048>
{
public:

	~Test_USB_Core_task() override
	{

	}

	void work() override;
};
