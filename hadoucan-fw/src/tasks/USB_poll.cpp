#include "tasks/USB_poll.hpp"

#include "Task_instances.hpp"

#include "freertos_cpp_util/logging/Global_logger.hpp"

using freertos_util::logging::LOG_LEVEL;

void USB_core_task::work()
{
	for(;;)
	{
		tud_task();

		taskYIELD();
	}
}

void USB_core_task::wait_for_usb_rx_avail()
{
	while( ! (RX_AVAIL_BIT & m_events.wait_bits(RX_AVAIL_BIT, true, true, portMAX_DELAY)) )
	{

	}
}
void USB_core_task::wait_for_usb_tx_complete()
{
	while( ! (TX_COMPL_BIT & m_events.wait_bits(TX_COMPL_BIT, true, true, portMAX_DELAY)) )
	{

	}
}

void USB_core_task::get_unique_id(std::array<uint32_t, 3>* const id)
{
	uint32_t volatile * const addr = reinterpret_cast<uint32_t*>(0x1FF1E800);

	std::copy_n(addr, 3, id->data());
}

void USB_core_task::get_unique_id_str(std::array<char, 25>* const id_str)
{
	//0x012345670123456701234567
	std::array<uint32_t, 3> id;
	get_unique_id(&id);

	snprintf(id_str->data(), id_str->size(), "%08" PRIX32 "%08" PRIX32 "%08" PRIX32, id[0], id[1], id[2]);
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
		"Suburban Marine, Inc.", // 1: Manufacturer
		"HadouCAN",              // 2: Product
		NULL,                    // 3: Serials will use unique ID if possible
		"HadouCAN CDC"           // 4: CDC Interface
	};

	void ascii_to_u16le(const size_t len, char const * const in, uint16_t* const out)
	{
		for(size_t i = 0; i < len; i++)
		{
			out[i+1] = in[i];
		}
	}

	static uint16_t desc_str_u16 [64 + 1];
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
				std::array<char, 25> id_str;
				USB_core_task::get_unique_id_str(&id_str);

				chr_count = id_str.size() - 1;

				ascii_to_u16le(chr_count, id_str.data(), desc_str_u16);

				break;
			}
			default:
			{
				if ( !(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0])) ) return NULL;

				const char *str = string_desc_arr[index];

				chr_count = std::min(strlen(str), sizeof(desc_str_u16) / sizeof(desc_str_u16[0]) - 1);

				ascii_to_u16le(chr_count, str, desc_str_u16);

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

	void tud_mount_cb(void)
	{

	}
	void tud_umount_cb(void)
	{

	}

	void tud_cdc_line_state_cb(uint8_t instance, bool dtr, bool rts)
	{
		usb_core_task.m_dtr.store(dtr);
		usb_core_task.m_rts.store(rts);
	}

	void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const* p_line_coding)
	{
		usb_core_task.m_bit_rate.store(p_line_coding->bit_rate);
		usb_core_task.m_stop_bits.store(p_line_coding->stop_bits);
		usb_core_task.m_parity.store(p_line_coding->parity);
		usb_core_task.m_data_bits.store(p_line_coding->data_bits);
	}

	// RX complete data available
	void tud_cdc_rx_cb(uint8_t itf)
	{
		usb_core_task.m_events.set_bits(USB_core_task::RX_AVAIL_BIT);
	}

	// TX complete space available
	void tud_cdc_tx_complete_cb(uint8_t itf)
	{
		usb_core_task.m_events.set_bits(USB_core_task::TX_COMPL_BIT);
	}
}
