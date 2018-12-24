#pragma once
#include "main.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"

#include "freertos_cpp_util/BSema_static.hpp"
#include "freertos_cpp_util/Task_static.hpp"
#include "freertos_cpp_util/Queue_static_pod.hpp"
#include "freertos_cpp_util/object_pool/Object_pool.hpp"

#include <atomic>

class USB_RX_task : public Task_static<1024>
{
public:
	class RX_buf
	{
	public:
		RX_buf()
		{
			reset();
		}

		void reset()
		{
			len = 0;
		}

		std::array<uint8_t, CDC_DATA_HS_OUT_PACKET_SIZE> buf;
		size_t len;
	};

	typedef Object_pool<RX_buf, 8> USB_rx_pool_type;
	typedef USB_rx_pool_type::unique_node_ptr USB_rx_buf_ptr;

	USB_RX_task();

	void handle_init_callback();
	void work() override;

	int8_t handle_rx_callback(uint8_t* in_buf, uint32_t in_buf_len);

protected:

	BSema_static m_init_complete;

	USB_rx_pool_type m_rx_buf_pool;

	//USB_MAX_EP0_SIZE
	//CDC_DATA_HS_OUT_PACKET_SIZE
	std::atomic<RX_buf*> m_active_buf;

	Queue_static_pod<RX_buf*, 8> m_full_buffers;
};