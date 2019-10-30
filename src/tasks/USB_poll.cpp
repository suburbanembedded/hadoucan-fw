#include "USB_poll.hpp"

#include "Task_instances.hpp"

void Test_USB_Core_task::work()
{
	for(;;)
	{
		usb_core.poll_driver();
		usb_core.poll_event_loop();

		// vTaskDelay(0);
		taskYIELD();
	}
}
