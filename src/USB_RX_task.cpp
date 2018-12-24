#include "USB_RX_task.hpp"

#include "hal_inst.hpp"
#include "uart1_printf.hpp"

#include "freertos_cpp_util/Critical_section.hpp"

#include <algorithm>
#include <cinttypes>

USB_RX_task::USB_RX_task()
{
	m_pool_buf = nullptr;
}

void USB_RX_task::handle_init_callback()
{
	USBD_CDC_SetTxBuffer(&hUsbDeviceHS, nullptr, 0);
	USBD_CDC_SetRxBuffer(&hUsbDeviceHS, m_buf);

	m_init_complete.give_from_isr();
}

void USB_RX_task::work()
{
	// m_rx_buf_pool.initialize();

	m_pool_buf = m_rx_buf_pool.allocate();

	Object_pool_node<RX_buf>* n_ptr = Object_pool_node<RX_buf>::get_this_from_val_ptr(m_pool_buf);
	uart1_print<64>("First alloc\r\n");
	uart1_print<64>("\tm_pool_buf            is 0x%" PRIXPTR "\r\n", m_pool_buf);
	uart1_print<64>("\t&m_rx_buf_pool        is 0x%" PRIXPTR "\r\n", &m_rx_buf_pool);
	uart1_print<64>("\tn_ptr                 is 0x%" PRIXPTR "\r\n", n_ptr);
	if(n_ptr)
	{
		uart1_print<64>("\tn_ptr->get_val_ptr()  is 0x%" PRIXPTR "\r\n", n_ptr->get_val_ptr());
		uart1_print<64>("\tn_ptr->get_pool_ptr() is 0x%" PRIXPTR "\r\n", n_ptr->get_pool_ptr());
	}

	MX_USB_DEVICE_Init();

	m_init_complete.take();

	for(;;)
	{
		//wait for read completion
		m_read_complete.take();

		HAL_GPIO_TogglePin(GPIOD, GREEN1_Pin);

		RX_buf* new_pool_buf = m_rx_buf_pool.allocate();

		Object_pool_node<RX_buf>* n_ptr = Object_pool_node<RX_buf>::get_this_from_val_ptr(new_pool_buf);
		uart1_print<64>("new alloc\r\n");
		uart1_print<64>("\tnew_pool_buf            is 0x%" PRIXPTR "\r\n", new_pool_buf);
		uart1_print<64>("\t&m_rx_buf_pool        is 0x%" PRIXPTR "\r\n", &m_rx_buf_pool);
		uart1_print<64>("\tn_ptr                 is 0x%" PRIXPTR "\r\n", n_ptr);
		if(n_ptr)
		{
			uart1_print<64>("\tn_ptr->get_val_ptr()  is 0x%" PRIXPTR "\r\n", n_ptr->get_val_ptr());
			uart1_print<64>("\tn_ptr->get_pool_ptr() is 0x%" PRIXPTR "\r\n", n_ptr->get_pool_ptr());
		}

		{
			Critical_section crit_sec;
			std::swap(m_pool_buf, new_pool_buf);
		}

		uart1_print<64>("start dealloc\r\n");

		m_rx_buf_pool.deallocate(new_pool_buf);

		uart1_print<64>("finish dealloc\r\n");

		// HAL_GPIO_TogglePin(GPIOD, RED1_Pin);
	}
}

int8_t USB_RX_task::handle_rx_callback(uint8_t* in_buf, uint32_t in_buf_len)
{
	//std::copy_n(in_buf, in_buf_len, m_pool_buf->buf.data());
	//m_pool_buf->len = in_buf_len;

	HAL_GPIO_TogglePin(GPIOD, RED1_Pin);

	// //swap front and back buffers
	USBD_CDC_SetRxBuffer(&hUsbDeviceHS, m_buf);
	USBD_CDC_ReceivePacket(&hUsbDeviceHS);

	m_read_complete.give_from_isr();

	return (USBD_OK);
}
