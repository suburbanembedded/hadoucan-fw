#include "USB_TX_task.hpp"

extern USBD_HandleTypeDef hUsbDeviceHS;

void USB_TX_task::work()
{
	// m_init_complete.take();
	
	for(;;)
	{
		vTaskDelay(500);
	}
}

void USB_TX_task::wait_tx_finish()
{
	USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceHS.pClassData;
	while(hcdc->TxState != 0)
	{
		vTaskDelay(1);
	}
}

uint8_t USB_TX_task::send_buffer(uint8_t* buf, uint16_t len)
{
	wait_tx_finish();
    
	USBD_CDC_SetTxBuffer(&hUsbDeviceHS, buf, len);
	uint8_t result = USBD_CDC_TransmitPacket(&hUsbDeviceHS);
	return result;
}