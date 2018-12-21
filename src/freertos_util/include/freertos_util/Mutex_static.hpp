#pragma once

#include "freertos_util/Mutex_base.hpp"

class Mutex_static : public Mutex_base
{
public:
	Mutex_static()
	{
		m_mutex = xSemaphoreCreateMutexStatic(&m_mutex_buf);
	}
	~Mutex_static() override
	{

	}
protected:
	StaticSemaphore_t m_mutex_buf;
};
