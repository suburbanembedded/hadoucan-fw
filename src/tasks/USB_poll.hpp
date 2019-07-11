#pragma once

#include "freertos_cpp_util/Task_static.hpp"

class Test_USB_Core_task : public Task_static<4096>
{
public:
	void work() override
	{
		for(;;)
		{
			usb_core.poll_driver();
			usb_core.poll_event_loop();

			vTaskDelay(0);
		}
	}
};