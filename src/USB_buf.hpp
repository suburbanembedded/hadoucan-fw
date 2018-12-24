#pragma once

#include "freertos_cpp_util/object_pool/Object_pool.hpp"

#include "usbd_cdc_if.h"

class USB_buf
{
public:
	USB_buf()
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

typedef Object_pool<USB_buf, 16> USB_rx_pool_type;
extern USB_rx_pool_type rx_buf_pool;

typedef Object_pool<USB_buf, 16> USB_tx_pool_type;
extern USB_tx_pool_type tx_buf_pool;