#pragma once

#include "Task_base.hpp"

class Task_heap : public Task_base
{
public:
	bool launch(const char* name, const size_t StackDepth, const UBaseType_t priority);
};