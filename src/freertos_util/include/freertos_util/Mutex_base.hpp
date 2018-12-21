#pragma once

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include <chrono>

class Mutex_base
{
public:

	Mutex_base()
	{
		m_mutex = nullptr;
	}

	virtual ~Mutex_base()
	{
		vSemaphoreDelete(m_mutex);
		m_mutex = nullptr;
	}

	void lock()
	{
		xSemaphoreTake(m_mutex, portMAX_DELAY);
	}

	bool try_lock()
	{
		return pdTRUE == xSemaphoreTake(m_mutex, 0);
	}

	template< class Rep, class Period >
	bool try_lock_for(const std::chrono::duration<Rep,Period>& duration)
	{
		std::chrono::milliseconds duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
		return try_lock_for_ticks( pdMS_TO_TICKS(duration_ms.count()) );
	}

	bool try_lock_for_ticks(const TickType_t ticks)
	{
		return pdTRUE == xSemaphoreTake(m_mutex, ticks);
	}

	void unlock()
	{
		xSemaphoreGive(m_mutex);
	}

	TaskHandle_t get_owner()
	{
		return xSemaphoreGetMutexHolder(m_mutex);
	}

protected:
	SemaphoreHandle_t m_mutex;
};
