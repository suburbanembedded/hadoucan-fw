/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "USB_poll.hpp"

#include "Task_instances.hpp"

#include "freertos_cpp_util/logging/Global_logger.hpp"

#include "tusb.h"

using freertos_util::logging::LOG_LEVEL;

namespace
{
	static const bool isr_mode = true;
}

void Test_USB_Core_task::work()
{
	for(;;)
	{
		tud_task();
		
		tud_cdc_write_flush();

		taskYIELD();
	}
}

extern "C"
{
	void OTG_FS_IRQHandler(void)
	{
		tusb_int_handler(0, true);
	}

	void OTG_HS_IRQHandler(void)
	{
		tusb_int_handler(1, true);
	}

	#define USB_VID   0x6666
	#define USB_PID   0x6666
	#define USB_BCD   0x0200

	#define ITF_NUM_CDC      0
	#define ITF_NUM_CDC_DATA 1

	#define EPNUM_CDC_NOTIF   0x81
	#define EPNUM_CDC_OUT     0x02
	#define EPNUM_CDC_IN      0x82

	enum {
		STRID_LANGID = 0,
		STRID_MANUFACTURER,
		STRID_PRODUCT,
		STRID_SERIAL
	};

	uint8_t const desc_fs_configuration[] =
	{
		TUD_CONFIG_DESCRIPTOR(1, 2, 0, TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN, 0x00, 100),
		TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 4, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, 64),
	};

	uint8_t const desc_hs_configuration[] =
	{
		TUD_CONFIG_DESCRIPTOR(1, 2, 0, TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN, 0x00, 100),
		TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 4, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, 512),
	};

	tusb_desc_device_t const desc_device = {
		.bLength            = sizeof(tusb_desc_device_t),
		.bDescriptorType    = TUSB_DESC_DEVICE,
		.bcdUSB             = USB_BCD,

		.bDeviceClass       = TUSB_CLASS_MISC,
		.bDeviceSubClass    = MISC_SUBCLASS_COMMON,
		.bDeviceProtocol    = MISC_PROTOCOL_IAD,

		.bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

		.idVendor           = USB_VID,
		.idProduct          = USB_PID,
		.bcdDevice          = 0x0100,

		.iManufacturer      = 0x01,
		.iProduct           = 0x02,
		.iSerialNumber      = 0x03,

		.bNumConfigurations = 0x01
	};

	uint8_t const * tud_descriptor_device_cb(void) {
		return (uint8_t const *) &desc_device;
	}

	uint8_t const * tud_descriptor_configuration_cb(uint8_t index)
	{
		uint8_t const * ret = nullptr;
		switch(tud_speed_get())
		{
			case TUSB_SPEED_HIGH:
			{
				ret = desc_hs_configuration;
				break;
			}
			case TUSB_SPEED_FULL:
			{
				ret = desc_fs_configuration;
				break;
			}
			default:
			{
				ret = nullptr;
				break;
			}
		}

		return ret;
	}

	char const *string_desc_arr[] =
	{
		(const char[]) { 0x09, 0x04 }, // 0: is supported language is English (0x0409)
		"SM",          // 1: Manufacturer
		"Hadoucan",    // 2: Product
		NULL,          // 3: Serials will use unique ID if possible
		"Hadoucan CDC" // 4: CDC Interface
	};

	static uint16_t desc_str_u16 [32 + 1];
	uint16_t const * tud_descriptor_string_cb(uint8_t index, uint16_t langid)
	{
		size_t chr_count = 0;

		switch ( index )
		{
			case STRID_LANGID:
			{
				memcpy(&desc_str_u16[1], string_desc_arr[0], 2);
				chr_count = 1;
				break;
			}
			case STRID_SERIAL:
			{
				chr_count = 0;
				break;
			}
			default:
			{
				if ( !(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0])) ) return NULL;

				const char *str = string_desc_arr[index];

				// Cap at max char
				chr_count = strlen(str);
				size_t const max_count = sizeof(desc_str_u16) / sizeof(desc_str_u16[0]) - 1; // -1 for string type
				if ( chr_count > max_count ) chr_count = max_count;

				// Convert ASCII string into UTF-16
				for ( size_t i = 0; i < chr_count; i++ ) {
					desc_str_u16[1 + i] = str[i];
				}
				break;
			}
		}

		desc_str_u16[0] = (uint16_t) ((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));

		return desc_str_u16;
	}

	tusb_desc_device_qualifier_t const desc_device_qualifier =
	{
	  .bLength            = sizeof(tusb_desc_device_qualifier_t),
	  .bDescriptorType    = TUSB_DESC_DEVICE_QUALIFIER,
	  .bcdUSB             = USB_BCD,

	  .bDeviceClass       = TUSB_CLASS_MISC,
	  .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
	  .bDeviceProtocol    = MISC_PROTOCOL_IAD,

	  .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
	  .bNumConfigurations = 0x01,
	  .bReserved          = 0x00
	};
	uint8_t const* tud_descriptor_device_qualifier_cb(void)
	{
		return (uint8_t const*) &desc_device_qualifier;
	}

	uint8_t desc_other_speed_config[TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN];
	uint8_t const* tud_descriptor_other_speed_configuration_cb(uint8_t index)
	{
		switch(tud_speed_get())
		{
			case TUSB_SPEED_FULL:
			{
				memcpy(desc_other_speed_config, desc_hs_configuration, TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN);
				break;
			}
			case TUSB_SPEED_HIGH:
			{
				memcpy(desc_other_speed_config, desc_fs_configuration, TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN);
				break;
			}
			default:
			{
				break;
			}
		}

		return desc_other_speed_config;
	}

	void tud_suspend_cb(bool remote_wakeup_en)
	{

	}

	void tud_resume_cb(void)
	{
		
	}
}
