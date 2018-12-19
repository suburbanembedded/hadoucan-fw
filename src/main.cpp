#include "main.h"
#include "cmsis_os.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"

#include "freertos_util/Task_static.hpp"

#include <array>
#include <algorithm>

extern USBD_HandleTypeDef hUsbDeviceHS;

class USB_task : public Task_static<256>
{
public:

  void work() override
  {
    MX_USB_DEVICE_Init();

    vTaskDelay(5000);

    for(;;)
    { 
      send_buffer((uint8_t*)"Hi\r\n", 4);
      vTaskDelay(500);
    }
  }

  void init_buffers()
  {
    USBD_CDC_SetTxBuffer(&hUsbDeviceHS, m_tx_buf.data(), 0);
    USBD_CDC_SetRxBuffer(&hUsbDeviceHS, m_rx_buf.data());
    USBD_CDC_ReceivePacket(&hUsbDeviceHS);
  }

  int8_t handle_rx_callback(uint8_t* Buf, uint32_t Len)
  {
    USBD_CDC_SetRxBuffer(&hUsbDeviceHS, m_rx_buf.data());
    USBD_CDC_ReceivePacket(&hUsbDeviceHS);
    return (USBD_OK);
  }

  void wait_tx_finish()
  {
    USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceHS.pClassData;
    while(hcdc->TxState != 0)
    {
      vTaskDelay(1);
    }
  }

protected:

  uint8_t send_buffer(uint8_t* buf, uint16_t len)
  {
    wait_tx_finish();
    
    USBD_CDC_SetTxBuffer(&hUsbDeviceHS, buf, len);
    uint8_t result = USBD_CDC_TransmitPacket(&hUsbDeviceHS);
    return result;
  }

  std::array<uint8_t, 2048> m_rx_buf;
  std::array<uint8_t, 2048> m_tx_buf;
};

USB_task usb_task;

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
    usb_task.init_buffers();
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
    return usb_task.handle_rx_callback(Buf, *Len);
  }

  int8_t CDC_Control_HS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
  {
    return (USBD_OK);
  }
}

void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_USART1_UART_Init(void);
void MX_FDCAN1_Init(void);
void MX_CRC_Init(void);
void MX_HASH_Init(void);
void MX_RTC_Init(void);
void MX_RNG_Init(void);

int main(void)
{
  SCB_EnableICache();

  SCB_EnableDCache();

  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_FDCAN1_Init();
  MX_CRC_Init();
  MX_HASH_Init();
  MX_RTC_Init();
  MX_RNG_Init();

  usb_task.launch("usb_task", 1);
  vTaskStartScheduler();
  
  for(;;)
  {

  }
}