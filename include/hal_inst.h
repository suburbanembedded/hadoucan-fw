

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32h7xx_hal.h"

#include "usb_device.h"
#include "usbd_cdc_if.h"

extern USBD_HandleTypeDef hUsbDeviceHS;
extern UART_HandleTypeDef huart1;
extern FDCAN_HandleTypeDef hfdcan1;
extern QSPI_HandleTypeDef hqspi;
extern TIM_HandleTypeDef htim3;

#ifdef __cplusplus
}
#endif


