#pragma once

#include "FreeRTOS.h"
#include "task.h"

class Critical_section_isr
{
public:
	Critical_section_isr()
	{
		m_mask = taskENTER_CRITICAL_FROM_ISR();
	}

	~Critical_section_isr()
	{
		taskEXIT_CRITICAL_FROM_ISR(m_mask);
	}
protected:
	UBaseType_t m_mask;
};