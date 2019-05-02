#pragma once

#include "freertos_cpp_util/object_pool/Object_pool.hpp"

#include "usbd_cdc_if.h"

#include "core_cm7.h"

template<size_t BUFLEN>
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
		int32_t inv_len = ((len + 31) / 32)*32;

		SCB_InvalidateDCache_by_Addr(inv_ptr, inv_len);	
	}

	void clean_cache()
	{
		uint32_t* inv_ptr = reinterpret_cast<uint32_t*>(reinterpret_cast<uint32_t>(buf.data()) & (~0x1FU));
		int32_t inv_len = ((len + 31) / 32)*32;

		SCB_CleanDCache_by_Addr(inv_ptr, inv_len);	
	}

	void clean_invalidate_cache()
	{
		uint32_t* inv_ptr = reinterpret_cast<uint32_t*>(reinterpret_cast<uint32_t>(buf.data()) & (~0x1FU));
		int32_t inv_len = ((len + 31) / 32)*32;

		SCB_CleanInvalidateDCache_by_Addr(inv_ptr, inv_len);	
	}

	alignas(32) std::array<uint8_t, BUFLEN> buf;
	size_t len;
};

typedef USB_buf<CDC_DATA_HS_OUT_PACKET_SIZE> USB_buf_rx;
typedef Object_pool<USB_buf_rx, 16> USB_rx_pool_type;
extern USB_rx_pool_type rx_buf_pool;

typedef USB_buf<CDC_DATA_HS_IN_PACKET_SIZE> USB_buf_tx;
typedef Object_pool<USB_buf_tx, 32> USB_tx_pool_type;
extern USB_tx_pool_type tx_buf_pool;