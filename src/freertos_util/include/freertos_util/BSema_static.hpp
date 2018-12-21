#pragma once

#include "freertos_util/Semaphore_base.hpp"

#include <atomic>

class BSema_static : public Semaphore_base
{
public:

	BSema_static()
	{
		m_sema = xSemaphoreCreateBinaryStatic(&m_sema_buf);
	}

	~BSema_static() override
	{
		
	}

	bool take()
	{
		std::atomic_thread_fence(std::memory_order_seq_cst);

		return pdTRUE == xSemaphoreTake(m_sema, portMAX_DELAY);
	}

	bool take_from_isr()
	{
		std::atomic_thread_fence(std::memory_order_seq_cst);

		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		const BaseType_t ret = xSemaphoreTakeFromISR(m_sema, &xHigherPriorityTaskWoken);
		
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

		return pdTRUE == ret;
	}

	bool give()
	{
		std::atomic_thread_fence(std::memory_order_seq_cst);

		return pdTRUE == xSemaphoreGive(m_sema);
	}

	bool give_from_isr()
	{
		std::atomic_thread_fence(std::memory_order_seq_cst);
		
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		const BaseType_t ret = xSemaphoreGiveFromISR(m_sema, &xHigherPriorityTaskWoken);

		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

		return pdTRUE == ret;
	}

protected:

	StaticSemaphore_t m_sema_buf;

};