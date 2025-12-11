#pragma once

#include "freertos_cpp_util/Task_static.hpp"

#include <atomic>

class LED_task : public Task_static<512>
{
public:

	LED_task();

	void work() override;

	//external interface
	void notify_can_rx();
	void notify_can_tx();

	void notify_usb_rx();
	void notify_usb_tx();

	//modes
	void set_mode_boot();
	void set_mode_normal();
	void set_mode_error();
	void set_mode_suspend();

protected:

	enum class LED_MODE
	{
		BOOT,
		NORMAL,
		ERROR,
		SUSPEND
	};
	std::atomic<LED_MODE> m_mode;

	static void handle_boot_mode();
	void handle_normal_mode();
	void handle_error_mode();
	static void handle_suspend_mode();

	static void all_off();
	static void all_on();

	std::atomic<bool> m_usb_activity;
	std::atomic<bool> m_can_rx_activity;
	std::atomic<bool> m_can_tx_activity;
};