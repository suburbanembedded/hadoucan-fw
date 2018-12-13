#pragma once

#include "freertos_util/Queue_static_pod.hpp"

#include "FreeRTOS.h"
#include "task.h"

#include <array>

template<typename T, size_t LEN>
class Queue_static : public Queue_template_base<T>
{
public:

	Queue_static()
	{
		
	}
//	~Queue_static() override
//	{
//
//	}

	bool push_back(const T& item)
	{
		return push_back(item, 0);
	}

	bool push_back(const T& item, const TickType_t xTicksToWait)
	{
		TickType_t ticks_left = xTicksToWait;
		TimeOut_t xTimeOut;
		vTaskSetTimeOutState( &xTimeOut );

		T* ptr = nullptr;
		if(!m_free_queue.pop_front(&ptr, ticks_left))
		{
			return false;
		}

		//recalculate how long to wait
		if(pdTRUE == xTaskCheckForTimeOut(&xTimeOut, &ticks_left))
		{
			ticks_left = 0;
		}

		//copy construct
		ptr->T(item);

		if(!m_alloc_queue.push_back(ptr, ticks_left))
		{
			return false;
		}

		return true;
	}

protected:

	typedef std::aligned_storage_t<sizeof(T), alignof(T)> Alligned_T;

	Queue_static_pod<Alligned_T*, LEN> m_free_queue;
	Queue_static_pod<T*, LEN> m_alloc_queue;

	std::array<Alligned_T, LEN> m_data_buf;
};
