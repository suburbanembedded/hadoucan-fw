#pragma once

#include "freertos_cpp_util/Task_static.hpp"

class Test_USB_Core_task : public Task_static<2048>
{
public:

	~Test_USB_Core_task() override
	{

	}

	void work() override;
};

class Test_USB_Driver_task : public Task_static<2048>
{
public:

	~Test_USB_Driver_task() override
	{

	}

	void work() override;
};

#include "freertos_cpp_util/BSema_static.hpp"
class Test_USB_CDC_task : public Task_static<2048>
{
public:

	~Test_USB_CDC_task() override
	{

	}

	void work() override;

	void notify_new_connection();

protected:

	BSema_static m_new_connection;

};
