#include "USB_RX_task.hpp"

#include <algorithm>

extern USBD_HandleTypeDef hUsbDeviceHS;

void USB_RX_task::work()
{
	m_init_complete.take();

	vTaskDelay(5000);

	//first init
	// m_active_buf_front = m_rx_buf_pool.allocate_unique();
	// m_active_buf_back = m_rx_buf_pool.allocate_unique();
	// m_active_buf_front->reset();
	// m_active_buf_back->reset();

 	USBD_CDC_SetTxBuffer(&hUsbDeviceHS, nullptr, 0);
	USBD_CDC_SetRxBuffer(&hUsbDeviceHS, m_buf.buf.data());
	USBD_CDC_ReceivePacket(&hUsbDeviceHS);

	for(;;)
	{
		//wait for read completion
		m_read_complete.take();

		// HAL_GPIO_TogglePin(GPIOD, GREEN1_Pin);

		// //handoff buffers
		// USB_rx_buf_ptr stash = std::move(m_active_buf_back);

		// //get new buffer
		// do
		// {
		// 	m_active_buf_back = m_rx_buf_pool.try_allocate_for_ticks_unique(1);
		// } while(!m_active_buf_back);

		// HAL_GPIO_TogglePin(GPIOD, RED1_Pin);
	}
}

int8_t USB_RX_task::handle_rx_callback(uint8_t* in_buf, uint32_t in_buf_len)
{
	// std::copy_n(in_buf, in_buf_len, m_active_buf_front->buf.data());
	// m_active_buf_front->len = in_buf_len;

	// //swap front and back buffers
	// HAL_GPIO_TogglePin(GPIOD, GREEN2_Pin);
	// std::swap(m_active_buf_back, m_active_buf_front);
	USBD_CDC_SetRxBuffer(&hUsbDeviceHS, m_buf.buf.data());
	USBD_CDC_ReceivePacket(&hUsbDeviceHS);

	m_read_complete.give_from_isr();

	return (USBD_OK);
}
