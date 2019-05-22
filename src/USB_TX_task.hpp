#pragma once

#include "USB_buf.hpp"

#include "main.h"

#include "freertos_cpp_util/BSema_static.hpp"
#include "freertos_cpp_util/Mutex_static.hpp"
#include "freertos_cpp_util/Task_static.hpp"
#include "freertos_cpp_util/object_pool/Object_pool.hpp"

class USB_TX_task : public Task_static<1024>
{
public:

	USB_TX_task();

	void handle_init_callback();

	void work() override;

	void wait_tx_finish();

	size_t queue_buffer(const uint8_t* buf, const size_t len, const TickType_t xTicksToWait);
	size_t queue_buffer_blocking(const uint8_t* buf, const size_t len);

	void notify_tx_complete_callback();

	bool is_tx_complete();

protected:

	uint8_t send_buffer(USB_buf_tx* const buf);

	static const uint32_t USB_HS_PACKET_WAIT_MS = 50;

	BSema_static m_init_complete;
	BSema_static m_tx_idle;

	Queue_static_pod<USB_buf_tx*, 32> m_pending_tx_buffers;
};