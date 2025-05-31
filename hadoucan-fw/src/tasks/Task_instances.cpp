#include "Task_instances.hpp"


LED_task led_task                 __attribute__(( section(".ram_d2_s1_noload") ));
Timesync_task timesync_task       __attribute__(( section(".ram_d2_s1_noload") ));
System_mon_task system_mon_task   __attribute__(( section(".ram_d2_s1_noload") ));;
Logging_task logging_task         __attribute__(( section(".ram_d2_s1_noload") ));

Main_task main_task                __attribute__(( section(".ram_dtcm_noload") ));
STM32_fdcan_rx stm32_fdcan_rx_task __attribute__ (( section(".ram_dtcm_noload") ));
USB_lawicel_task usb_lawicel_task  __attribute__(( section(".ram_dtcm_noload") ));

USB_rx_buffer_task usb_rx_buffer_task __attribute__(( section(".ram_dtcm_noload") ));
USB_tx_buffer_task usb_tx_buffer_task __attribute__(( section(".ram_dtcm_noload") ));

Test_USB_Core_task test_usb_core   __attribute__(( section(".ram_dtcm_noload") ));
