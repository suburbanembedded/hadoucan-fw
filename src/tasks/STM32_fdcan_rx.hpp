#pragma once

#include "freertos_cpp_util/Task_static.hpp"
#include "freertos_cpp_util/BSema_static.hpp"
#include "freertos_cpp_util/Queue_static_pod.hpp"

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_fdcan.h"

#include <atomic>
#include <functional>

class STM32_fdcan_rx : public Task_static<1024>
{
public:

	STM32_fdcan_rx()
	{
		m_last_fifo_msg_lost_check = 0;

		//technically in C++11 if using the non-default constructor
		//we haveto use these init functions
		std::atomic_init(&m_can_fifo0_full, false);
		std::atomic_init(&m_can_fifo0_msg_lost, 0U);

		std::atomic_init(&m_can_fifo1_full, false);
		std::atomic_init(&m_can_fifo1_msg_lost, 0U);
	}

	typedef std::function<bool (const std::string str)> PacketRXCallback;

	void set_packet_callback(const PacketRXCallback& func)
	{
		m_rx_callback = func;
	}
	
	void set_can_instance(FDCAN_GlobalTypeDef* const can)
	{
		m_fdcan = can;
	}

	void set_can_handle(FDCAN_HandleTypeDef* const can_handle)
	{
		m_fdcan_handle = can_handle;
	}

	void work();

	struct CAN_fd_packet
	{
		FDCAN_RxHeaderTypeDef rxheader;
		std::array<uint8_t, 64> data;
	};

	void can_fifo0_callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs);
	void can_fifo1_callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs);

protected:



	static bool append_packet_type(const FDCAN_RxHeaderTypeDef& rxheader, std::string* const s);
	static bool append_packet_id(const FDCAN_RxHeaderTypeDef& rxheader, std::string* const s);
	static bool append_packet_data(const CAN_fd_packet& rxheader, std::string* const s);

	FDCAN_GlobalTypeDef* m_fdcan;
	FDCAN_HandleTypeDef* m_fdcan_handle;

	//HW FIFO is 64 elements
	//Could have FIFO0 and FIFO1
	//set to 150% of FIFO0 + FIFO1
	Queue_static_pod<CAN_fd_packet, 192> m_can_fd_queue;

	PacketRXCallback m_rx_callback;

	TickType_t m_last_fifo_msg_lost_check;

	std::atomic_bool m_can_fifo0_full;
	std::atomic_uint m_can_fifo0_msg_lost;

	std::atomic_bool m_can_fifo1_full;
	std::atomic_uint m_can_fifo1_msg_lost;
};

extern STM32_fdcan_rx stm32_fdcan_rx_task;