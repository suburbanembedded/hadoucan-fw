#include "main.h"
#include "cmsis_os.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"

#include "uart1_printf.hpp"

#include "USB_RX_task.hpp"
#include "USB_TX_task.hpp"
#include "USB_rx_buffer_task.hpp"
#include "USB_tx_buffer_task.hpp"

#include "lawicel/Lawicel_parser_stm32.hpp"
#include "STM32_fdcan_rx.hpp"
#include "STM32_fdcan_tx.hpp"

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
    uart1_printf<64>("called ~foo on 0x%" PRIXPTR "\r\n", this);
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
			uart1_printf<64>("a is ok\r\n");

			Object_pool_node<foo>* n_ptr = Object_pool_node<foo>::get_this_from_val_ptr(a);

			uart1_printf<64>("\ta                     is 0x%" PRIXPTR "\r\n", a);
			uart1_printf<64>("\t&m_pool               is 0x%" PRIXPTR "\r\n", &m_pool);
			uart1_printf<64>("\tn_ptr                 is 0x%" PRIXPTR "\r\n", n_ptr);
			uart1_printf<64>("\tn_ptr->get_val_ptr()  is 0x%" PRIXPTR "\r\n", n_ptr->get_val_ptr());
			uart1_printf<64>("\tn_ptr->get_pool_ptr() is 0x%" PRIXPTR "\r\n", n_ptr->get_pool_ptr());

      uart1_printf<64>("a.v1 is %d\r\n", a->m_v1);
      uart1_printf<64>("a.v2 is %d\r\n", a->m_v2);
		}
		if(b)
		{
			uart1_printf<64>("b is ok\r\n");

			Object_pool_node<foo>* n_ptr = Object_pool_node<foo>::get_this_from_val_ptr(b);

			uart1_printf<64>("\tb                     is 0x%" PRIXPTR "\r\n", b);
			uart1_printf<64>("\t&m_pool               is 0x%" PRIXPTR "\r\n", &m_pool);
			uart1_printf<64>("\tn_ptr                 is 0x%" PRIXPTR "\r\n", n_ptr);
			uart1_printf<64>("\tn_ptr->get_val_ptr()  is 0x%" PRIXPTR "\r\n", n_ptr->get_val_ptr());
			uart1_printf<64>("\tn_ptr->get_pool_ptr() is 0x%" PRIXPTR "\r\n", n_ptr->get_pool_ptr());
			
      uart1_printf<64>("b.v1 is %d\r\n", b->m_v1);
      uart1_printf<64>("b.v2 is %d\r\n", b->m_v2);
    }
    if(c)
    {
      uart1_printf<64>("c.v1 is %d\r\n", c->m_v1);
      uart1_printf<64>("c.v2 is %d\r\n", c->m_v2);
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

USB_rx_buffer_task usb_rx_buffer_task;
USB_tx_buffer_task usb_tx_buffer_task;

class USB_lawicel_task : public Task_static<1024>
{
public:

  USB_lawicel_task()
  {
    m_usb_tx_buffer = nullptr;
  }

  static bool usb_input_drop(uint8_t c)
  {
    switch(c)
    {
      // case '\r':
      //   return true;
      case '\n':
      {
        return true;
      }
      default:
      {
        return false;
      }
    }

    return false;
  }

  void set_usb_tx(USB_tx_buffer_task* const usb_tx_buffer)
  {
    m_usb_tx_buffer = usb_tx_buffer;
  }

  bool write_string_usb(const char* str)
  {
    m_usb_tx_buffer->write(str);
    return true;
  }

  void work() override
  {
    m_can.set_can(FDCAN1);
    m_parser.set_can(&m_can);
    m_parser.set_write_string_func(
      std::bind(&USB_lawicel_task::write_string_usb, this, std::placeholders::_1)
      );

    std::function<bool(void)> has_line_pred = std::bind(&USB_rx_buffer_task::has_line, &usb_rx_buffer_task);
    
    //a null terminated string
    //maybe using std::string is a better idea, or a stringstream....
    std::vector<uint8_t> usb_line;
    usb_line.reserve(128);

    for(;;)
    {
      {
        uart1_log<64>(LOG_LEVEL::TRACE, "USB_lawicel_task", "wait(lock, has_line_pred)");
        std::unique_lock<Mutex_static> lock(usb_rx_buffer_task.get_mutex());
        usb_rx_buffer_task.get_cv().wait(lock, has_line_pred);
        uart1_log<64>(LOG_LEVEL::TRACE, "USB_lawicel_task", "woke");

        if(!usb_rx_buffer_task.get_line(&usb_line))
        {
          continue;
        }
      }

      //drop what usb_input_drop says we should drop
      auto end_it = std::remove_if(usb_line.begin(), usb_line.end(), &usb_input_drop);
      usb_line.erase(end_it, usb_line.end());

      //drop lines that are now empty
      if(strnlen((char*)usb_line.data(), usb_line.size()) == 0)
      {
        uart1_log<64>(LOG_LEVEL::WARN, "USB_lawicel_task", "Empty line");
        continue;
      }

      //drop lines that are only '\r'
      if(usb_line.front() == '\r')
      {
        uart1_log<64>(LOG_LEVEL::WARN, "USB_lawicel_task", "Line only contains \\r");
        continue;
      }

      uart1_log<64>(LOG_LEVEL::TRACE, "USB_lawicel_task", "got line: [%s]", usb_line.data());

      //we unlock lock so buffering can continue

      //process line
      if(!m_parser.parse_string((char*)usb_line.data()))
      {
        uart1_log<64>(LOG_LEVEL::ERROR, "USB_lawicel_task", "parse error");
      }
      else
      {
        uart1_log<64>(LOG_LEVEL::TRACE, "USB_lawicel_task", "ok");
      }
    }
  }

protected:
  STM32_fdcan_tx m_can;

  Lawicel_parser_stm32 m_parser;

  USB_tx_buffer_task* m_usb_tx_buffer;
};
USB_lawicel_task usb_lawicel_task;

STM32_fdcan_rx stm32_fdcan_rx;

extern "C"
{
  FDCAN_HandleTypeDef hfdcan1;

  USBD_CDC_HandleTypeDef usb_cdc_class_data;

  void* USBD_cdc_class_malloc(size_t size)
  {
    if(size != sizeof(usb_cdc_class_data))
    {
      return nullptr;
    }

    return &usb_cdc_class_data;
  }

  void USBD_cdc_class_free(void* ptr)
  {

  }

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

  static char USB_SERIAL_NUMBER[25] = {0};
  char* get_usb_serial_number()
  {
    return USB_SERIAL_NUMBER;
  }
  void set_usb_serial_number(char id_str[25])
  {
    snprintf(USB_SERIAL_NUMBER, 25, "%s", id_str);
  }
}

void get_unique_id(std::array<uint32_t, 3>* id)
{
  volatile uint32_t* addr = reinterpret_cast<uint32_t*>(0x1FF1E800);

  std::copy_n(addr, 3, id->data());
}

void get_unique_id_str(std::array<char, 25>* id_str)
{
  //0x012345670123456701234567
  std::array<uint32_t, 3> id;
  get_unique_id(&id);

  snprintf(id_str->data(), id_str->size(), "%08" PRIX32 "%08" PRIX32 "%08" PRIX32, id[0], id[1], id[2]);
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

{
  //errata 2.2.9
  volatile uint32_t* AXI_TARG7_FN_MOD = 
  reinterpret_cast<uint32_t*>(
    0x51000000 + 
    0x1108 + 
    0x1000*7U
    );

  uint32_t AXI_TARGx_FN_MOD_READ_ISS_OVERRIDE  = 0x00000001;
  uint32_t AXI_TARGx_FN_MOD_WRITE_ISS_OVERRIDE = 0x00000002;

  SET_BIT(*AXI_TARG7_FN_MOD, AXI_TARGx_FN_MOD_READ_ISS_OVERRIDE);
}

  SCB_EnableICache();

  // SCB_EnableDCache();

  HAL_Init();

  set_all_gpio_low_power();

  SystemClock_Config();

  {
    std::array<char, 25> id_str;
    get_unique_id_str(&id_str);

    set_usb_serial_number(id_str.data());
  }

  MX_GPIO_Init();
  MX_USART1_UART_Init();
  //MX_FDCAN1_Init();
  MX_CRC_Init();
  MX_HASH_Init();
  MX_RTC_Init();
  MX_RNG_Init();

  	if(0)
	{
		/*Configure GPIO pin : PA8 */
		GPIO_InitTypeDef GPIO_InitStruct = {0};
		GPIO_InitStruct.Pin = GPIO_PIN_8;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
		GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSE, RCC_MCODIV_1);
	}

	{
		std::array<char, 25> id_str;
		get_unique_id_str(&id_str);
    uart1_log<64>(LOG_LEVEL::INFO, "main", "Initialing");
		uart1_log<64>(LOG_LEVEL::INFO, "main", "P/N: SM-1301");
		uart1_log<64>(LOG_LEVEL::INFO, "main", "S/N: %s", id_str.data());
	}

  //init
  usb_rx_buffer_task.set_usb_rx(&usb_rx_task);
  usb_tx_buffer_task.set_usb_tx(&usb_tx_task);
  stm32_fdcan_rx.set_usb_tx(&usb_tx_buffer_task);
  usb_lawicel_task.set_usb_tx(&usb_tx_buffer_task);

  //can RX
  stm32_fdcan_rx_task.launch("stm32_fdcan_rx", 1);
  
  //protocol state machine
  usb_lawicel_task.launch("usb_lawicel", 2);

  //process usb packets
  usb_rx_buffer_task.launch("usb_rx_buf", 4);
  usb_tx_buffer_task.launch("usb_tx_buf", 5);

  //actually send usb packets on the wire
  usb_rx_task.launch("usb_rx", 3);
  usb_tx_task.launch("usb_tx", 4);

  uart1_log<64>(LOG_LEVEL::INFO, "main", "Ready");

  vTaskStartScheduler();
  
  for(;;)
  {

  }
}
