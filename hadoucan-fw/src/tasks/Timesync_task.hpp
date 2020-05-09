#pragma once

#include "freertos_cpp_util/Task_static.hpp"

class Timesync_task : public Task_static<512>
{
public:

	void work() override;

	bool oc_config();

	bool ic_config();
};