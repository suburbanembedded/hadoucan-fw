#pragma once

#include "USB_buf.hpp"

#include "freertos_cpp_util/BSema_static.hpp"
#include "freertos_cpp_util/Task_static.hpp"
#include "freertos_cpp_util/Queue_static_pod.hpp"
#include "freertos_cpp_util/object_pool/Object_pool.hpp"

#include <atomic>

class USB_RX_task : public Task_static<1024>
{
public:

	typedef USB_rx_pool_type::unique_node_ptr USB_rx_buf_ptr;

	USB_RX_task();

	void handle_init_callback();
	void work() override;

	int8_t handle_rx_callback(uint8_t* in_buf, uint32_t in_buf_len);

	USB_rx_buf_ptr get_rx_buffer();

protected:

	BSema_static m_init_complete;

	BSema_static m_rx_complete;

	//USB_MAX_EP0_SIZE
	//CDC_DATA_HS_OUT_PACKET_SIZE
	std::atomic<USB_buf*> m_active_buf;

	Queue_static_pod<USB_buf*, 16> m_full_buffers;
};
