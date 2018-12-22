/**
 * @brief Base semaphore class
 * @author Jacob Schloss <jacob@schloss.io>
 * @copyright Copyright (c) 2018 Jacob Schloss. All rights reserved.
 * @license Licensed under the terms of the 3-Clause BSD license. See License for details
*/

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