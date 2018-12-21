#pragma once

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

class Semaphore_base
{
public:

	Semaphore_base()
	{
		m_sema = nullptr;
	}

	virtual ~Semaphore_base()
	{
		vSemaphoreDelete(m_sema);
		m_sema = nullptr;
	}

protected:

	SemaphoreHandle_t m_sema;
};