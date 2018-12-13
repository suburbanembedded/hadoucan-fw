#pragma once


#include "FreeRTOS.h"
#include "queue.h"

class Queue_base
{
public:

	Queue_base();
	virtual ~Queue_base();

	void clear()
	{
		xQueueReset(m_queue);
	}

	bool full() const
	{
		return 0 == uxQueueSpacesAvailable(m_queue);
	}

	bool empty() const
	{
		return 0 == uxQueueMessagesWaiting(m_queue);	
	}

protected:

	QueueHandle_t m_queue;

};

template <typename T>
class Queue_template_base : public Queue_base
{
	bool push_back(const T& item)
	{
		return push_back(item, 0);
	}

	bool push_back(const T& item, const TickType_t xTicksToWait)
	{
		return xQueueSendToBack(m_queue, &item, xTicksToWait) == pdTRUE;
	}

	bool push_front(const T& item)
	{
		return push_front(item, 0);
	}

	bool push_front(const T& item, const TickType_t xTicksToWait)
	{
		return xQueueSendToFront(m_queue, &item, xTicksToWait) == pdTRUE;
	}

	bool push_front_isr(const T& item)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		
		bool ret = push_front_isr(item, &xHigherPriorityTaskWoken);

		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

		return ret;
	}

	bool push_front_isr(const T& item, BaseType_t* const pxHigherPriorityTaskWoken)
	{
		return pdTRUE = xQueueSendToFrontFromISR(m_queue, &item, pxHigherPriorityTaskWoken) == pdTRUE;
	}

	bool push_back_isr(const T& item)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		
		bool ret = push_back_isr(item, &xHigherPriorityTaskWoken);

		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

		return ret;
	}

	bool push_back_isr(const T& item, BaseType_t* const pxHigherPriorityTaskWoken)
	{
		return pdTRUE = xQueueSendToBackFromISR(m_queue, &item, pxHigherPriorityTaskWoken) == pdTRUE;
	}
};
