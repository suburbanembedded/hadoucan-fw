#pragma once

#include "LED_task.hpp"
#include "Logging_task.hpp"
#include "Main_task.hpp"
#include "STM32_fdcan_rx.hpp"
#include "System_mon_task.hpp"
#include "Timesync_task.hpp"
#include "USB_lawicel_task.hpp"
#include "USB_poll.hpp"

extern USB_rx_buffer_task usb_rx_buffer_task;
extern USB_tx_buffer_task usb_tx_buffer_task;

extern Test_USB_Core_task   test_usb_core;
extern Test_USB_Driver_task test_usb_drvr;

extern LED_task led_task;
extern USB_lawicel_task usb_lawicel_task;
extern Timesync_task timesync_task;
extern Logging_task logging_task;
extern System_mon_task system_mon_task;

extern STM32_fdcan_rx stm32_fdcan_rx_task;

extern Main_task main_task;
