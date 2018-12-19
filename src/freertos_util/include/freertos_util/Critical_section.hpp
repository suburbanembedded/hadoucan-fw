#pragma once

#include "FreeRTOS.h"
#include "task.h"

class Critical_section
{
public:
	Critical_section()
	{
		taskENTER_CRITICAL();
	}

	~Critical_section()
	{
		taskEXIT_CRITICAL();
	}
};