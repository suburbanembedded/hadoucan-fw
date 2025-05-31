#pragma once

#include "freertos_cpp_util/Task_static.hpp"
#include "freertos_cpp_util/Mutex_static.hpp"
#include "freertos_cpp_util/Condition_variable.hpp"

#include <algorithm>
#include <deque>
#include <functional>
#include <vector>

#include <cstring>

//aggregate small writes
class USB_tx_buffer_task : public Task_static<1024>
{
public:

  //if an insertion would put us over this limit, block until there is space
  static constexpr size_t BUFFER_HIGH_WATERMARK = 1024 * 16;

	USB_tx_buffer_task()
	{
		m_usb_tx_pkt_watermark = 512;
		m_usb_tx_delay         = 50;
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

	//insert [first, last) into the internal buffer
	//will block until space is available
	template<typename InputIt>
	void write(InputIt first, InputIt last)
	{
		const size_t len = std::distance(first, last);

		write(first, last, len);
	}

	//insert string into the internal buffer
	//will block until space is available
	void write(const char* str)
	{
		const size_t len = strlen(str);

		write(str, str + len, len);
	}

protected:

	//insert [first, last) into the internal buffer
	//len is used to cap ram usage
	//will block until space is available
	template<typename InputIt>
	void write(InputIt first, InputIt last, const size_t len)
	{
		std::function<bool()> enqueue_pred = std::bind(&USB_tx_buffer_task::m_tx_buf_has_space, this, len);

		//wait until m_tx_buf_has_space says we can insert our stuff
		{
			std::unique_lock<Mutex_static> lock(m_tx_buf_mutex);
			
			m_tx_buf_drain_condvar.wait(lock, std::cref(enqueue_pred));

			m_tx_buf.insert(m_tx_buf.end(), first, last);
		}

		//notify the io thread, which will decide if it wants to create a buffer
		m_tx_buf_condvar.notify_one();
	}

	//predicate for enqueue to m_tx_buf_condvar
	//m_tx_buf_mutex must be locked
	bool m_tx_buf_has_space(const size_t len) const
	{
		//false if keep waiting
		return (m_tx_buf.size() + len) < BUFFER_HIGH_WATERMARK;
	}

	//predicate for m_tx_buf_condvar write to usb
	//m_tx_buf_mutex must be locked
	bool has_buffer() const
	{
		if(m_tx_buf.empty())
		{
			//no point waking other thread
			//that thread should self-wake to send a 0 len packet after a bit, once
			return false;
		}

		if(m_tx_buf.size() >= m_usb_tx_pkt_watermark)
		{
			//yay, we have a watermarks worth of data
			return true;
		}

		return false;
	}

	std::deque<uint8_t> m_tx_buf;
	Mutex_static m_tx_buf_mutex;
	Condition_variable m_tx_buf_condvar;
	Condition_variable m_tx_buf_drain_condvar;

	//params
	unsigned m_usb_tx_pkt_watermark;
	unsigned m_usb_tx_delay;
};
