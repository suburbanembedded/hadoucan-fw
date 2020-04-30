#include "USB_poll.hpp"

#include "Task_instances.hpp"

#include "libusb_dev_cpp/class/cdc/cdc_notification.hpp"

#include "freertos_cpp_util/logging/Global_logger.hpp"

using freertos_util::logging::LOG_LEVEL;

namespace
{
	static const bool isr_mode = true;
}

void Test_USB_Core_task::work()
{
	// TODO: switch to isr mode
	if(isr_mode)
	{
		HAL_NVIC_SetPriority(OTG_HS_IRQn, 5, 0);
		HAL_NVIC_EnableIRQ(OTG_HS_IRQn);
	}

	for(;;)
	{
		usb_core.wait_event_loop();
		taskYIELD();
	}
}

extern "C"
{
	void OTG_HS_IRQHandler(void)
	{
		{
			// const char msg[] = "OTG_HS_IRQHandler";
			// HAL_UART_Transmit(&huart1, reinterpret_cast<uint8_t*>(const_cast<char*>(msg)), strlen(msg), -1);
		}

		usb_core.poll_driver();
	}
}

void Test_USB_Driver_task::work()
{

	for(;;)
	{
		if(!isr_mode)
		{
			usb_core.poll_driver();
			taskYIELD();
		}
		else
		{
			suspend();
		}
	}
}


void Test_USB_CDC_task::work()
{
	freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();

	NETWORK_CONNECTION_NOTIFICATION net_conn_notify;
	net_conn_notify.set_connection(true);

	for(;;)
	{
		if(!m_new_connection.take())
		{
			continue;
		}

		Buffer_adapter_base* tx_buf = usb_driver.wait_tx_buffer(0x82);
		tx_buf->reset();
		if(!net_conn_notify.serialize(tx_buf))
		{
			logger->log(freertos_util::logging::LOG_LEVEL::FATAL, "Test_USB_CDC_task", "net_conn_notify serialize failed");
			suspend();
		}

		logger->log(freertos_util::logging::LOG_LEVEL::INFO, "Test_USB_CDC_task", "sending net_conn_notify");
		if(!usb_driver.enqueue_tx_buffer(0x82, tx_buf))
		{
			logger->log(freertos_util::logging::LOG_LEVEL::ERROR, "Test_USB_CDC_task", "failed to enqueue tx buffer");
		}
	}
}

void Test_USB_CDC_task::notify_new_connection()
{
	if(!m_new_connection.give())
	{
		freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();
		logger->log(freertos_util::logging::LOG_LEVEL::FATAL, "Test_USB_CDC_task", "failed to give m_new_connection");
	}
}