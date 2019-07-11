#include "Task_instances.hpp"

USB_rx_buffer_task usb_rx_buffer_task __attribute__(( section(".ram_dtcm_noload") ));
USB_tx_buffer_task usb_tx_buffer_task __attribute__(( section(".ram_dtcm_noload") ));

Test_USB_Core_task test_usb_core __attribute__(( section(".ram_dtcm_noload") ));

LED_task led_task __attribute__(( section(".ram_dtcm_noload") ));
USB_lawicel_task usb_lawicel_task __attribute__(( section(".ram_dtcm_noload") ));
Timesync_task timesync_task __attribute__(( section(".ram_dtcm_noload") ));

Main_task main_task __attribute__(( section(".ram_dtcm_noload") ));
