#include "USB_poll.hpp"

#include "Task_instances.hpp"

void Test_USB_Core_task::work()
{
	// TODO: switch to isr mode
	// HAL_NVIC_SetPriority(OTG_HS_IRQn, 5, 0);
	// HAL_NVIC_EnableIRQ(OTG_HS_IRQn);

	for(;;)
	{
		usb_core.poll_driver();
		usb_core.poll_event_loop();

		// vTaskDelay(0);
		taskYIELD();
	}
}

extern "C"
{
	void OTG_HS_IRQHandler(void)
	{
		// {
		// 	const char msg[] = "OTG_HS_IRQHandler";
		// 	// HAL_UART_Transmit(&huart1, reinterpret_cast<uint8_t*>(const_cast<char*>(msg)), strlen(msg), -1);
		// }

		// usb_core.poll_driver();
	}
}
