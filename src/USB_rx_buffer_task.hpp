#pragma once

#include "USB_RX_task.hpp"

#include "freertos_cpp_util/Task_static.hpp"
#include "freertos_cpp_util/Mutex_static.hpp"
#include "freertos_cpp_util/Condition_variable.hpp"

#include <algorithm>
#include <deque>
#include <vector>

class USB_rx_buffer_task : public Task_static<1024>
{
public:

  USB_rx_buffer_task()
  {
    m_usb_rx_task = nullptr;
  }

  void set_usb_rx(USB_RX_task* const usb_rx_task)
  {
    m_usb_rx_task = usb_rx_task;
  }

  void work() override;

  //you must hold a lock on m_rx_buf_mutex
  bool has_line()
  {
    auto it = std::find(m_rx_buf.begin(), m_rx_buf.end(), '\r');
    
    return it != m_rx_buf.end();
  }

  //you must hold a lock on m_rx_buf_mutex
  bool get_line(std::vector<uint8_t>* out_line)
  {
    out_line->clear();

    const auto cr_it = std::find(m_rx_buf.begin(), m_rx_buf.end(), '\r');

    if(cr_it == m_rx_buf.end())
    {
      return false;
    }

    //copy the [begin, \r]
    const auto cr_next_it = std::next(cr_it);
    out_line->insert(out_line->begin(), m_rx_buf.begin(), cr_next_it);
    out_line->push_back('\0');

    //erase the [begin, \r]
    m_rx_buf.erase(m_rx_buf.begin(), cr_next_it);

    return true;
  }

  Mutex_static& get_mutex()
  {
    return m_rx_buf_mutex;
  }

  Condition_variable& get_cv()
  {
    return m_rx_buf_condvar;
  }

protected:
  Mutex_static m_rx_buf_mutex;
  Condition_variable m_rx_buf_condvar;
  std::deque<uint8_t> m_rx_buf;

  USB_RX_task* m_usb_rx_task;
};