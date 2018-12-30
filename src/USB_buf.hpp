#pragma once

#include "freertos_cpp_util/object_pool/Object_pool.hpp"

#include "usbd_cdc_if.h"

#include "core_cm7.h"

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

	void invalidate_cache()
	{
		uint32_t* inv_ptr = reinterpret_cast<uint32_t*>(reinterpret_cast<uint32_t>(buf.data()) & (~0x1FU));
		size_t inv_len = ((len + 31U) / 32U)*32U;

		SCB_InvalidateDCache_by_Addr(inv_ptr, inv_len);	
	}

	void flush_cache()
	{
		uint32_t* inv_ptr = reinterpret_cast<uint32_t*>(reinterpret_cast<uint32_t>(buf.data()) & (~0x1FU));
		size_t inv_len = ((len + 31U) / 32U)*32U;

		SCB_CleanDCache_by_Addr(inv_ptr, inv_len);	
	}

	void flush_invalidate_cache()
	{
		uint32_t* inv_ptr = reinterpret_cast<uint32_t*>(reinterpret_cast<uint32_t>(buf.data()) & (~0x1FU));
		size_t inv_len = ((len + 31U) / 32U)*32U;

		SCB_CleanInvalidateDCache_by_Addr(inv_ptr, inv_len);	
	}

	std::array<uint8_t, CDC_DATA_HS_OUT_PACKET_SIZE> buf;
	size_t len;
};

typedef Object_pool<USB_buf, 16> USB_rx_pool_type;
extern USB_rx_pool_type rx_buf_pool;

typedef Object_pool<USB_buf, 32> USB_tx_pool_type;
extern USB_tx_pool_type tx_buf_pool;