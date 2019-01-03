#pragma once

#include "USB_tx_buffer_task.hpp"

#include "freertos_cpp_util/Task_static.hpp"
#include "freertos_cpp_util/BSema_static.hpp"
#include "freertos_cpp_util/Queue_static_pod.hpp"

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_fdcan.h"

class STM32_fdcan_rx : public Task_static<1024>
{
public:

	STM32_fdcan_rx()
	{
		m_usb_tx_buffer = nullptr;
	}

	void set_usb_tx(USB_tx_buffer_task* const usb_tx_buffer)
	{
		m_usb_tx_buffer = usb_tx_buffer;
	}

	void work();

	struct CAN_fd_packet
	{
		FDCAN_RxHeaderTypeDef rxheader;
		std::array<uint8_t, 64> data;
	};

	bool insert_packet_isr(CAN_fd_packet& pk);

protected:

	Queue_static_pod<CAN_fd_packet, 64> m_can_fd_queue;

	USB_tx_buffer_task* m_usb_tx_buffer;

};

extern STM32_fdcan_rx stm32_fdcan_rx_task;