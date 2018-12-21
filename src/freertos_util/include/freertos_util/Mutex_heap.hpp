#pragma once

#include "freertos_util/Mutex_base.hpp"

class Mutex_heap : public Mutex_base
{
public:
	Mutex_heap()
	{
		m_mutex = xSemaphoreCreateMutex();
	}
	~Mutex_heap() override
	{

	}
protected:

};
