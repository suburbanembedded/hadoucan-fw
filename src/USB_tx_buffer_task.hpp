#pragma once

#include "USB_TX_task.hpp"

#include "freertos_cpp_util/Task_static.hpp"
#include "freertos_cpp_util/Mutex_static.hpp"
#include "freertos_cpp_util/Condition_variable.hpp"

#include <algorithm>
#include <deque>
#include <vector>

//aggragate small writes
class USB_tx_buffer_task : public Task_static<1024>
{
public:

	USB_tx_buffer_task()
	{
		m_usb_tx_task = nullptr;
	}

	void set_usb_tx(USB_TX_task* const usb_tx_task)
	{
		m_usb_tx_task = usb_tx_task;
	}

	void work() override;

	Mutex_static& get_mutex()
	{
		return m_tx_buf_mutex;
	}

	Condition_variable& get_cv()
	{
		return m_tx_buf_condvar;
	}	

	bool has_buffer()
	{
		if(m_tx_buf.size() >= 512)
		{
			//yay, full USB HS packet
			return true;
		}
		else if(m_tx_buf.empty())
		{
			//no point waking other thread
			//that thread should self-wake to send a 0 len packet after a bit, once
			return false;
		}

		//pdTRUE on timeout, send a partial packet since non-empty
		BaseType_t ret = xTaskCheckForTimeOut(&m_tx_timeout, &m_tx_timeout_ticks_left);

		return ret == pdTRUE;
	}

	//insert [first, last) into the internal buffer
	template<typename InputIt>
	void write(InputIt first, InputIt last)
	{
		{
			std::unique_lock<Mutex_static> lock(m_tx_buf_mutex);
			m_tx_buf.insert(m_tx_buf.end(), first, last);
		}

		//notify the io thread, which will decide if it wants to create a buffer
		m_tx_buf_condvar.notify_one();
	}

protected:

	std::deque<uint8_t> m_tx_buf;
	Mutex_static m_tx_buf_mutex;
	Condition_variable m_tx_buf_condvar;

	TimeOut_t  m_tx_timeout;
	TickType_t m_tx_timeout_ticks_left;

	USB_TX_task* m_usb_tx_task;
};
