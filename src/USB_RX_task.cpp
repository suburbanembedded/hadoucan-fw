#include "USB_RX_task.hpp"

#include <algorithm>

extern USBD_HandleTypeDef hUsbDeviceHS;

USB_RX_task::USB_RX_task()
{
	m_pool_buf = nullptr;
}

void USB_RX_task::handle_init_callback()
{
	USBD_CDC_SetTxBuffer(&hUsbDeviceHS, nullptr, 0);
	USBD_CDC_SetRxBuffer(&hUsbDeviceHS, m_buf.buf.data());
	// USBD_CDC_ReceivePacket(&hUsbDeviceHS);
	// m_init_complete.give_from_isr();
	// m_init_complete.give();
}

void USB_RX_task::work()
{
	MX_USB_DEVICE_Init();

	// m_init_complete.take();

	//first init
	// m_active_buf_front = m_rx_buf_pool.allocate_unique();
	// m_active_buf_back = m_rx_buf_pool.allocate_unique();
	// m_active_buf_front->reset();
	// m_active_buf_back->reset();

	for(;;)
	{
		//wait for read completion
		m_read_complete.take();

		HAL_GPIO_TogglePin(GPIOD, GREEN1_Pin);

		m_rx_buf_pool.deallocate(m_pool_buf);

		HAL_GPIO_WritePin(GPIOD, GREEN2_Pin, GPIO_PIN_SET);
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
	m_pool_buf = m_rx_buf_pool.try_allocate_isr();
	if(!m_pool_buf)
	{
		HAL_GPIO_WritePin(GPIOD, RED2_Pin, GPIO_PIN_SET);
	}

	// std::copy_n(in_buf, in_buf_len, m_pool_buf->buf.data());
	// m_pool_buf->len = in_buf_len;
	HAL_GPIO_TogglePin(GPIOD, RED1_Pin);

	// //swap front and back buffers
	USBD_CDC_SetRxBuffer(&hUsbDeviceHS, m_buf.buf.data());
	USBD_CDC_ReceivePacket(&hUsbDeviceHS);

	m_read_complete.give_from_isr();

	return (USBD_OK);
}
