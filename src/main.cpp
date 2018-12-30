#include "main.h"
#include "cmsis_os.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"

#include "uart1_printf.hpp"

#include "USB_RX_task.hpp"
#include "USB_TX_task.hpp"

#include "lawicel/Lawicel_parser_stm32.hpp"

#include "freertos_cpp_util/Task_static.hpp"
#include "freertos_cpp_util/BSema_static.hpp"
#include "freertos_cpp_util/Mutex_static.hpp"
#include "freertos_cpp_util/Condition_variable.hpp"
#include "freertos_cpp_util/object_pool/Object_pool.hpp"

#include <array>
#include <deque>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <cinttypes>

#if 1
class foo
{
public:
  foo()
  {
    m_v1 = 0; 
    m_v2 = 0;
  }
  foo(int x)
  {
   m_v1 = x; 
   m_v2 = 2;
  }
  ~foo()
  {
    uart1_print<64>("called ~foo on 0x%" PRIXPTR "\r\n", this);
  }
  int m_v1;
  int m_v2;
};

class Pool_test_task : public Task_static<1024>
{
public:

  void work() override
  {
    for(;;)
    {
		HAL_UART_Transmit(&huart1, (uint8_t*)"test\r\n", 6, 100);
		vTaskDelay(500);

		foo* a = m_pool.allocate();
		foo* b = m_pool.try_allocate_for_ticks(3, 4);
		foo* c = m_pool.try_allocate_for(std::chrono::milliseconds(5), 5);

		if(a)
		{
			uart1_print<64>("a is ok\r\n");

			Object_pool_node<foo>* n_ptr = Object_pool_node<foo>::get_this_from_val_ptr(a);

			uart1_print<64>("\ta                     is 0x%" PRIXPTR "\r\n", a);
			uart1_print<64>("\t&m_pool               is 0x%" PRIXPTR "\r\n", &m_pool);
			uart1_print<64>("\tn_ptr                 is 0x%" PRIXPTR "\r\n", n_ptr);
			uart1_print<64>("\tn_ptr->get_val_ptr()  is 0x%" PRIXPTR "\r\n", n_ptr->get_val_ptr());
			uart1_print<64>("\tn_ptr->get_pool_ptr() is 0x%" PRIXPTR "\r\n", n_ptr->get_pool_ptr());

      uart1_print<64>("a.v1 is %d\r\n", a->m_v1);
      uart1_print<64>("a.v2 is %d\r\n", a->m_v2);
		}
		if(b)
		{
			uart1_print<64>("b is ok\r\n");

			Object_pool_node<foo>* n_ptr = Object_pool_node<foo>::get_this_from_val_ptr(b);

			uart1_print<64>("\tb                     is 0x%" PRIXPTR "\r\n", b);
			uart1_print<64>("\t&m_pool               is 0x%" PRIXPTR "\r\n", &m_pool);
			uart1_print<64>("\tn_ptr                 is 0x%" PRIXPTR "\r\n", n_ptr);
			uart1_print<64>("\tn_ptr->get_val_ptr()  is 0x%" PRIXPTR "\r\n", n_ptr->get_val_ptr());
			uart1_print<64>("\tn_ptr->get_pool_ptr() is 0x%" PRIXPTR "\r\n", n_ptr->get_pool_ptr());
			
      uart1_print<64>("b.v1 is %d\r\n", b->m_v1);
      uart1_print<64>("b.v2 is %d\r\n", b->m_v2);
    }
    if(c)
    {
      uart1_print<64>("c.v1 is %d\r\n", c->m_v1);
      uart1_print<64>("c.v2 is %d\r\n", c->m_v2);
		}


		Object_pool<foo, 16>::free(a);
		Object_pool<foo, 16>::free(b);
		Object_pool<foo, 16>::free(c);
		//m_pool.deallocate(a);


    }
  }

protected:
  Object_pool<foo, 16> m_pool;
};

Pool_test_task pool_test_task;
#endif
USB_RX_task usb_rx_task;
USB_TX_task usb_tx_task;


class USB_echo_task : public Task_static<1024>
{
public:

  void work() override
  {
    for(;;)
    {
      USB_RX_task::USB_rx_buf_ptr in_buf = usb_rx_task.get_rx_buffer();

      usb_tx_task.queue_buffer_blocking(in_buf->buf.data(), in_buf->len);
    }
  }

};
USB_echo_task usb_echo_task;

/*
class USB_rx_buffer_task : public Task_static<1024>
{
public:

  void work() override
  {
    for(;;)
    {
      USB_RX_task::USB_rx_buf_ptr in_buf = usb_rx_task.get_rx_buffer();

      {
        std::unique_lock<Mutex_static> lock(m_rx_buf_mutex);
        m_rx_buf.insert(m_rx_buf.end(), in_buf->buf.data(), in_buf->buf.data() + in_buf->len);
      }

      m_rx_buf_condvar.notify_one();
    }
  }

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

    //include the \r
    const auto cr_next_it = std::next(cr_it);
    out_line->insert(out_line->begin(), m_rx_buf.begin(), cr_next_it);
    out_line->push_back('\0');

    //erase the \r
    m_rx_buf.erase(cr_next_it);

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

};
USB_rx_buffer_task usb_rx_buffer_task;

class USB_lawicel_task : public Task_static<1024>
{
public:

  void work() override
  {
    std::function<bool(void)> has_line_pred = std::bind(&USB_rx_buffer_task::has_line, &usb_rx_buffer_task);
    
    std::vector<uint8_t> out_line;
    out_line.reserve(128);

    for(;;)
    {
      //allocate line
      {
        std::unique_lock<Mutex_static> lock(usb_rx_buffer_task.get_mutex());

        usb_rx_buffer_task.get_cv().wait(lock, has_line_pred);

        if(!usb_rx_buffer_task.get_line(&out_line))
        {
          continue;
        }
      }

      uart1_print<64>("got line: %s\r\n", out_line.data());

      //we unlock lock so buffering can continue

      //process line
    }
  }

protected:
  Lawicel_parser_stm32 m_parser;
};

USB_lawicel_task usb_lawicel_task;
*/

extern "C"
{
  int8_t CDC_Init_HS(void);
  int8_t CDC_DeInit_HS(void);
  int8_t CDC_Control_HS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
  int8_t CDC_Receive_HS(uint8_t* pbuf, uint32_t *Len);
  void CDC_TX_Cmpl_HS(void);

  USBD_CDC_ItfTypeDef USBD_Interface_fops_HS =
  {
    CDC_Init_HS,
    CDC_DeInit_HS,
    CDC_Control_HS,
    CDC_Receive_HS,
    CDC_TX_Cmpl_HS
  };

  int8_t CDC_Init_HS(void)
  {
    usb_rx_task.handle_init_callback();
    usb_tx_task.handle_init_callback();
    // usb_rx_task.launch("usb_rx", 3);
    // usb_tx_task.launch("usb_tx", 2);
    return (USBD_OK);
  }

  int8_t CDC_DeInit_HS(void)
  {
    /* USER CODE BEGIN 9 */
    return (USBD_OK);
    /* USER CODE END 9 */
  }

  int8_t CDC_Receive_HS(uint8_t* Buf, uint32_t *Len)
  {
    return usb_rx_task.handle_rx_callback(Buf, *Len);
  }

  void CDC_TX_Cmpl_HS(void)
  {
    usb_tx_task.notify_tx_complete_callback();
  }

  int8_t CDC_Control_HS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
  {
    return (USBD_OK);
  }
}

void set_gpio_low_power(GPIO_TypeDef* const gpio)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_All;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  // GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(gpio, &GPIO_InitStruct);
}

void set_all_gpio_low_power()
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOJ_CLK_ENABLE();
  __HAL_RCC_GPIOK_CLK_ENABLE();
  
  set_gpio_low_power(GPIOA);
  set_gpio_low_power(GPIOB);
  set_gpio_low_power(GPIOC);
  set_gpio_low_power(GPIOD);
  set_gpio_low_power(GPIOE);
  set_gpio_low_power(GPIOF);
  set_gpio_low_power(GPIOG);
  set_gpio_low_power(GPIOH);
  set_gpio_low_power(GPIOI);
  set_gpio_low_power(GPIOJ);
  set_gpio_low_power(GPIOK);

  __HAL_RCC_GPIOA_CLK_DISABLE();
  __HAL_RCC_GPIOB_CLK_DISABLE();
  __HAL_RCC_GPIOC_CLK_DISABLE();
  __HAL_RCC_GPIOD_CLK_DISABLE();
  __HAL_RCC_GPIOE_CLK_DISABLE();
  __HAL_RCC_GPIOF_CLK_DISABLE();
  __HAL_RCC_GPIOG_CLK_DISABLE();
  __HAL_RCC_GPIOH_CLK_DISABLE();
  __HAL_RCC_GPIOI_CLK_DISABLE();
  __HAL_RCC_GPIOJ_CLK_DISABLE();
  __HAL_RCC_GPIOK_CLK_DISABLE();
}

extern int RTOS_RAM_START;
extern int RTOS_ROM_START;
extern int RTOS_ROM_SIZE;

int main(void)
{

  SCB_EnableICache();

  // SCB_EnableDCache();

  HAL_Init();

  set_all_gpio_low_power();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_FDCAN1_Init();
  MX_CRC_Init();
  MX_HASH_Init();
  MX_RTC_Init();
  MX_RNG_Init();

  // pool_test_task.launch("pool_test", 1);
  // usb_rx_buffer_task.launch("usb_rx_buf", 1);
  // usb_lawicel_task.launch("usb_lawicel", 1);

  usb_echo_task.launch("usb_echo", 1);
  usb_rx_task.launch("usb_rx", 3);
  usb_tx_task.launch("usb_tx", 2);
  vTaskStartScheduler();
  
  for(;;)
  {

  }
}
