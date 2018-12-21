#include "main.h"
#include "cmsis_os.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"

#include "freertos_util/Task_static.hpp"
#include "freertos_util/BSema_static.hpp"

#include <array>
#include <algorithm>

extern USBD_HandleTypeDef hUsbDeviceHS;

class USB_tx_task : public Task_static<256>
{
public:
  USB_tx_task() : len(0)
  {

  }

  void work() override
  {

  	for(;;)
  	{
		if(m_sync.take())
		{
			send_buffer(m_tx_buf.data(), len);
			send_buffer(m_tx_buf.data(), 0);
		}
  	}
  }

//protected:

  void wait_tx_finish()
  {
    USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceHS.pClassData;
    while(hcdc->TxState != 0)
    {
      vTaskDelay(1);
    }
  }

  uint8_t send_buffer(uint8_t* buf, uint16_t len)
  {
    wait_tx_finish();
    
    USBD_CDC_SetTxBuffer(&hUsbDeviceHS, buf, len);
    uint8_t result = USBD_CDC_TransmitPacket(&hUsbDeviceHS);
    return result;
  }

  std::array<uint8_t, 2048> m_tx_buf;
  volatile size_t len;

  BSema_static m_sync;
};

class USB_rx_task : public Task_static<256>
{
public:

  USB_rx_task(USB_tx_task* tx) : m_tx(tx), len(0)
  {
	
  }

  void work() override
  {
    MX_USB_DEVICE_Init();

    vTaskDelay(5000);

    for(;;)
    { 
      //wait for data
      m_sync.take();

      HAL_GPIO_TogglePin(GPIOD, GREEN1_Pin);

      if(len != 0)
      {
      	HAL_GPIO_TogglePin(GPIOD, GREEN2_Pin);

      

      //wait for tx complete
	  m_tx->wait_tx_finish();

	  //copy, notify
      std::copy_n(m_rx_buf.data(), len, m_tx->m_tx_buf.data());
      m_tx->len = len;
      m_tx->m_sync.give();

      }

      USBD_CDC_SetRxBuffer(&hUsbDeviceHS, m_rx_buf.data());
      USBD_CDC_ReceivePacket(&hUsbDeviceHS);
    }
  }

  void init_buffers()
  {
    USBD_CDC_SetTxBuffer(&hUsbDeviceHS, nullptr, 0);
    USBD_CDC_SetRxBuffer(&hUsbDeviceHS, m_rx_buf.data());
    USBD_CDC_ReceivePacket(&hUsbDeviceHS);
  }

  int8_t handle_rx_callback(uint8_t* Buf, uint32_t Len)
  {
  	len = Len;
  	m_sync.give_from_isr();
    return (USBD_OK);
  }

protected:

  USB_tx_task* m_tx;

  std::array<uint8_t, 2048> m_rx_buf;
  volatile size_t len;
  
  BSema_static m_sync;
};

USB_tx_task usb_tx_task;
USB_rx_task usb_rx_task(&usb_tx_task);

extern "C"
{
  int8_t CDC_Init_HS(void);
  int8_t CDC_DeInit_HS(void);
  int8_t CDC_Control_HS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
  int8_t CDC_Receive_HS(uint8_t* pbuf, uint32_t *Len);

  USBD_CDC_ItfTypeDef USBD_Interface_fops_HS =
  {
    CDC_Init_HS,
    CDC_DeInit_HS,
    CDC_Control_HS,
    CDC_Receive_HS
  };

  int8_t CDC_Init_HS(void)
  {
    usb_rx_task.init_buffers();
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

  int8_t CDC_Control_HS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
  {
    return (USBD_OK);
  }
}

int main(void)
{
  SCB_EnableICache();

  //SCB_EnableDCache();

  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_FDCAN1_Init();
  MX_CRC_Init();
  MX_HASH_Init();
  MX_RTC_Init();
  MX_RNG_Init();

  usb_rx_task.launch("usb_rx", 1);
  usb_tx_task.launch("usb_tx", 2);
  vTaskStartScheduler();
  
  for(;;)
  {

  }
}