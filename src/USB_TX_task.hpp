#pragma once
#include "main.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"

#include "freertos_cpp_util/BSema_static.hpp"
#include "freertos_cpp_util/Task_static.hpp"
#include "freertos_cpp_util/object_pool/Object_pool.hpp"

class USB_TX_task : public Task_static<512>
{
public:

	void handle_init_callback()
	{
		m_init_complete.give_from_isr();
	}
	void work() override;

	void wait_tx_finish();

	uint8_t send_buffer(uint8_t* buf, uint16_t len);

protected:

	BSema_static m_init_complete;

};