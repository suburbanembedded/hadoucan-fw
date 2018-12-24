#include "freertos_cpp_util/Mutex_static.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32h7xx_hal.h"

#include "usb_device.h"
#include "usbd_cdc_if.h"

extern USBD_HandleTypeDef hUsbDeviceHS;
extern UART_HandleTypeDef huart1;

#ifdef __cplusplus
}
#endif

extern Mutex_static m_uart1_mutex;
