#pragma once

#include "FreeRTOS.h"
#include "queue.h"

#include <type_traits>

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
public:

	virtual bool pop_front(T* const item) = 0;

	virtual bool pop_front(T* const item, const TickType_t xTicksToWait) = 0;

	virtual bool push_back(const T& item) = 0;

	virtual bool push_back(const T& item, const TickType_t xTicksToWait) = 0;

	virtual bool push_front(const T& item) = 0;

	virtual bool push_front(const T& item, const TickType_t xTicksToWait) = 0;

	virtual bool push_front_isr(const T& item) = 0;

	virtual bool push_front_isr(const T& item, BaseType_t* const pxHigherPriorityTaskWoken) = 0;

	virtual bool push_back_isr(const T& item) = 0;

	virtual bool push_back_isr(const T& item, BaseType_t* const pxHigherPriorityTaskWoken) = 0;
};

template <typename T>
class Queue_template_base_pod : public Queue_template_base<T>
{
public:

	static_assert(std::is_pod<T>::value, "T must be POD");

	bool pop_front(T* const item) override
	{
		return pop_front(item, 0);
	}

	bool pop_front(T* const item, const TickType_t xTicksToWait) override
	{
		return xQueueReceive(this->m_queue, item, xTicksToWait) == pdTRUE;
	}

	bool push_back(const T& item) override
	{
		return push_back(item, 0);
	}

	bool push_back(const T& item, const TickType_t xTicksToWait) override
	{
		return xQueueSendToBack(this->m_queue, &item, xTicksToWait) == pdTRUE;
	}

	bool push_front(const T& item) override
	{
		return push_front(item, 0);
	}

	bool push_front(const T& item, const TickType_t xTicksToWait) override
	{
		return xQueueSendToFront(this->m_queue, &item, xTicksToWait) == pdTRUE;
	}

	bool push_front_isr(const T& item) override
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		
		bool ret = push_front_isr(item, &xHigherPriorityTaskWoken);

		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

		return ret;
	}

	bool push_front_isr(const T& item, BaseType_t* const pxHigherPriorityTaskWoken) override
	{
		return pdTRUE == xQueueSendToFrontFromISR(this->m_queue, &item, pxHigherPriorityTaskWoken);
	}

	bool push_back_isr(const T& item) override
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		
		bool ret = push_back_isr(item, &xHigherPriorityTaskWoken);

		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

		return ret;
	}

	bool push_back_isr(const T& item, BaseType_t* const pxHigherPriorityTaskWoken) override
	{
		return pdTRUE == xQueueSendToBackFromISR(this->m_queue, &item, pxHigherPriorityTaskWoken);
	}
};
