#pragma once

#include "freertos_cpp_util/Task_static.hpp"

class LED_task : public Task_static<512>
{
public:

	void work() override;
};