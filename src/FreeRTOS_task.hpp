#pragma once

#include "FreeRTOS_task_base.hpp"

class FreeRTOS_task : public FreeRTOS_task_base
{
public:
	bool launch(const char* name, const size_t StackDepth, const UBaseType_t priority);
};