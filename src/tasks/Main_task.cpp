#include "Main_task.hpp"

#include "global_app_inst.hpp"

#include "Task_instances.hpp"

#include "libusb_dev_cpp/class/cdc/cdc_desc.hpp"

#include "freertos_cpp_util/logging/Global_logger.hpp"

#include "hal_inst.h"
#include "stm32h7xx_hal.h"

#include <cinttypes>

USB_core         usb_core   __attribute__(( section(".ram_dtcm_noload") ));
CDC_class        usb_cdc    __attribute__(( section(".ram_dtcm_noload") ));
stm32_h7xx_otghs2 usb_driver __attribute__(( section(".ram_dtcm_noload") ));
EP_buffer_mgr_freertos<1, 8, 64,  32> usb_ep0_buffer __attribute__(( section(".ram_d2_s2_noload") ));
EP_buffer_mgr_freertos<3, 4, 512, 32> usb_tx_buffer __attribute__(( section(".ram_d2_s2_noload") ));
EP_buffer_mgr_freertos<3, 4, 512, 32> usb_rx_buffer __attribute__(( section(".ram_d2_s2_noload") ));

using freertos_util::logging::LOG_LEVEL;

namespace
{
	bool can_rx_to_lawicel(const std::string& str)
	{
		return usb_lawicel_task.get_lawicel()->queue_rx_packet(str);
	}
}

void Main_task::work()
{
	{
		freertos_util::logging::Global_logger::set(&logging_task.get_logger());
		freertos_util::logging::Global_logger::get()->set_sev_mask_level(freertos_util::logging::LOG_LEVEL::INFO);
	}

	freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();

	{
		CAN_USB_app::get_unique_id_str(&usb_id_str);
		logger->log(LOG_LEVEL::INFO, "main", "Initialing");
		logger->log(LOG_LEVEL::INFO, "main", "CAN FD <-> USB Adapter");
		logger->log(LOG_LEVEL::INFO, "main", "P/N: SM-1301");
		logger->log(LOG_LEVEL::INFO, "main", "S/N: %s", usb_id_str.data());
	}
	{
		const uint32_t idcode = DBGMCU->IDCODE;
		const uint16_t rev_id = (idcode & 0xFFFF0000) >> 16;
		const uint16_t dev_id = (idcode & 0x000007FF);

		if(dev_id == 0x450)
		{
			logger->log(LOG_LEVEL::INFO, "main", "Dev ID STM32H7xx (42, 43/53, 50)");
		}
		else
		{
			logger->log(LOG_LEVEL::WARN, "main", "Unk dev ID");
		}

		switch(rev_id)
		{
			case 0x1001:
			{
				logger->log(LOG_LEVEL::INFO, "main", "Silicon rev Z");
				logger->log(LOG_LEVEL::WARN, "main", "This silicon revision is not supported");
				break;
			}
			case 0x1003:
			{
				logger->log(LOG_LEVEL::INFO, "main", "Silicon rev Y");
				break;
			}
			case 0x2001:
			{
				logger->log(LOG_LEVEL::INFO, "main", "Silicon rev X");
				logger->log(LOG_LEVEL::WARN, "main", "This silicon revision is not supported");
				break;
			}
			case 0x2003:
			{
				logger->log(LOG_LEVEL::INFO, "main", "Silicon rev V");
				logger->log(LOG_LEVEL::WARN, "main", "This silicon revision is not supported, but will be soon");
				break;
			}
			default:
			{
				logger->log(LOG_LEVEL::WARN, "main", "Silicon rev unknown");
				logger->log(LOG_LEVEL::WARN, "main", "This silicon revision is not supported");
				break;
			}
		}
	}

	mount_fs();
	load_config();

	//load info
	CAN_USB_app_config::Config_Set config_struct;
	can_usb_app.get_config(&config_struct);
	CAN_USB_app_bitrate_table bitrate_table;
	can_usb_app.get_bitrate_tables(&bitrate_table);

	//update log level to config
	{
		freertos_util::logging::Global_logger::get()->set_sev_mask_level(config_struct.log_level);
	}

	//timesync processing
	//currently config then no-op
	timesync_task.launch("timesync", 1);

	//led blink/strobe
	led_task.launch("led", 1);

	if(!init_usb())
	{
		logger->log(LOG_LEVEL::ERROR, "main", "USB init failed");
	}
	
	//USB polling
	test_usb_core.launch("usb_core", 1);
	test_usb_cdc.launch("usb_cdc", 1);
	test_usb_drvr.launch("usb_drvr", 1);

	//CPU load & stack info
	system_mon_task.launch("SysMon", 2);

	//start logging_task
	logging_task.launch("logging", 3);
	
	can_usb_app.get_can_tx().set_can_instance(FDCAN1);
	can_usb_app.get_can_tx().set_can_handle(&hfdcan1);

	//init usb
	usb_rx_buffer_task.set_usb_driver(&usb_driver);
	usb_tx_buffer_task.set_usb_driver(&usb_driver);

	usb_lawicel_task.set_can_tx(&can_usb_app.get_can_tx());
	usb_lawicel_task.set_usb_tx(&usb_tx_buffer_task);
	usb_lawicel_task.set_usb_rx(&usb_rx_buffer_task);

	//TODO: refactor can handle init
	hfdcan1.Instance = FDCAN1;
	stm32_fdcan_rx_task.set_packet_callback(&can_rx_to_lawicel);
	stm32_fdcan_rx_task.set_can_instance(FDCAN1);
	stm32_fdcan_rx_task.set_can_handle(&hfdcan1);

	//can RX
	stm32_fdcan_rx_task.launch("stm32_fdcan_rx", 4);

	//protocol state machine
	usb_lawicel_task.launch("usb_lawicel", 5);

	//process usb packets
	usb_rx_buffer_task.launch("usb_rx_buf", 6);
	usb_tx_buffer_task.launch("usb_tx_buf", 7);

	if(config_struct.auto_startup)
	{
		if(!can_usb_app.get_can_tx().open())
		{
			logger->log(LOG_LEVEL::ERROR, "main", "CAN was requested to auto-start, but it failed");
		}
	}

	led_task.set_mode_normal();
	logger->log(LOG_LEVEL::INFO, "main", "Ready");

	for(;;)
	{
		vTaskSuspend(nullptr);
	}
}

bool Main_task::init_usb()
{
	freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();
	using freertos_util::logging::LOG_LEVEL;

	logger->log(LOG_LEVEL::INFO, "main", "usb_driver.set_ep0_buffer");
	usb_driver.set_ep0_buffer(&usb_ep0_buffer);

	logger->log(LOG_LEVEL::INFO, "main", "usb_driver.set_tx_buffer");
	usb_driver.set_tx_buffer(&usb_tx_buffer);
	
	logger->log(LOG_LEVEL::INFO, "main", "usb_driver.set_rx_buffer");
	usb_driver.set_rx_buffer(&usb_rx_buffer);

	logger->log(LOG_LEVEL::INFO, "main", "usb_driver.initialize");
	if(!usb_driver.initialize())
	{
		return false;	
	}

	//lifetime mgmt of some of these is broken
	{
		Device_descriptor dev_desc;
		dev_desc.bcdUSB = USB_common::build_bcd(2, 0, 0);
		// dev_desc.bDeviceClass    = static_cast<uint8_t>(USB_common::CLASS_DEF::CLASS_PER_INTERFACE);
		// dev_desc.bDeviceSubClass = static_cast<uint8_t>(USB_common::SUBCLASS_DEF::SUBCLASS_NONE);
		dev_desc.bDeviceClass    = 0x02;
		dev_desc.bDeviceSubClass = 0x02;
		dev_desc.bDeviceProtocol = static_cast<uint8_t>(USB_common::PROTO_DEF::PROTO_NONE);
		// dev_desc.bMaxPacketSize0 = m_driver->get_ep0_config().size;
		dev_desc.bMaxPacketSize0 = 64;
		dev_desc.idVendor  = 0x0483;
		dev_desc.idProduct = 0x5740;
		// dev_desc.idVendor  = 0xFFFF;
		// dev_desc.idProduct = 0x0001;
		dev_desc.bcdDevice = USB_common::build_bcd(1, 0, 0);
		dev_desc.iManufacturer      = 1;
		dev_desc.iProduct           = 2;
		dev_desc.iSerialNumber      = 3;
		dev_desc.bNumConfigurations = 1;

		usb_desc_table.set_device_descriptor(dev_desc, 0);
	}
	{
		//9 byte ea
		Interface_descriptor desc;
		desc.bInterfaceNumber   = 0;
		desc.bAlternateSetting  = 0;
		desc.bNumEndpoints      = 1;
		desc.bInterfaceClass    = static_cast<uint8_t>(CDC::COMM_INTERFACE_CLASS_CODE);
		desc.bInterfaceSubClass = static_cast<uint8_t>(CDC::COMM_INTERFACE_SUBCLASS_CODE::ACM);
		desc.bInterfaceProtocol = static_cast<uint8_t>(CDC::COMM_CLASS_PROTO_CODE::V250);
		// desc.bInterfaceProtocol = static_cast<uint8_t>(CDC::COMM_CLASS_PROTO_CODE::NONE);
		desc.iInterface         = 5;
		usb_desc_table.set_interface_descriptor(desc, 0);
	}
	{
		Interface_descriptor desc;
		desc.bInterfaceNumber   = 1;
		desc.bAlternateSetting  = 0;
		desc.bNumEndpoints      = 2;
		desc.bInterfaceClass    = static_cast<uint8_t>(CDC::DATA_INTERFACE_CLASS_CODE);
		desc.bInterfaceSubClass = static_cast<uint8_t>(CDC::DATA_INTERFACE_SUBCLASS_CODE);
		desc.bInterfaceProtocol = static_cast<uint8_t>(CDC::DATA_INTERFACE_PROTO_CODE::NONE);
		desc.iInterface         = 6;
		usb_desc_table.set_interface_descriptor(desc, 1);
	}
	{
		//7 byte ea
		Endpoint_descriptor desc;
		desc.bEndpointAddress = 0x00 | 0x01;
		desc.bmAttributes     = static_cast<uint8_t>(Endpoint_descriptor::ATTRIBUTE_TRANSFER::BULK);
		desc.wMaxPacketSize   = 512;
		desc.bInterval        = 0;

		usb_desc_table.set_endpoint_descriptor(desc, desc.bEndpointAddress);

		desc.bEndpointAddress = 0x80 | 0x01;
		desc.bmAttributes     = static_cast<uint8_t>(Endpoint_descriptor::ATTRIBUTE_TRANSFER::BULK);
		desc.wMaxPacketSize   = 512;
		desc.bInterval        = 0;
		usb_desc_table.set_endpoint_descriptor(desc, desc.bEndpointAddress);

		desc.bEndpointAddress = 0x80 | 0x02;
		// desc.bmAttributes     = static_cast<uint8_t>(Endpoint_descriptor::ATTRIBUTE_TRANSFER::BULK);
		desc.bmAttributes     = static_cast<uint8_t>(Endpoint_descriptor::ATTRIBUTE_TRANSFER::INTERRUPT);
		desc.wMaxPacketSize   = 8;
		desc.bInterval        = 8;//for HS, period is 2^(bInterval-1) * 125 us, so 8 -> 16ms
		usb_desc_table.set_endpoint_descriptor(desc, desc.bEndpointAddress);
	}

	{
		//configuration 1
		Config_desc_table::Config_desc_ptr desc_ptr = std::make_shared<Configuration_descriptor>();
		desc_ptr->wTotalLength = 0;//updated later
		desc_ptr->bNumInterfaces = 2;
		desc_ptr->bConfigurationValue = 1;
		desc_ptr->iConfiguration = 4;
		desc_ptr->bmAttributes = static_cast<uint8_t>(Configuration_descriptor::ATTRIBUTES::NONE);
		desc_ptr->bMaxPower = Configuration_descriptor::ma_to_maxpower(150);

		usb_desc_table.set_config_descriptor(desc_ptr, 0);
	}
	{
		std::shared_ptr<String_descriptor_zero> desc_ptr = std::make_shared<String_descriptor_zero>();

		const static String_descriptor_zero::LANGID lang[] = {String_descriptor_zero::LANGID::ENUS};
		desc_ptr->assign(lang, 1);

		usb_desc_table.set_string_descriptor(desc_ptr, String_descriptor_zero::LANGID::NONE, 0);
	}
	{
		String_descriptor_base desc;
		desc.assign("Suburban Marine, Inc.");
		usb_desc_table.set_string_descriptor(desc, String_descriptor_zero::LANGID::ENUS, 1);
	}
	{
		String_descriptor_base desc;
		desc.assign("SM-1301");
		usb_desc_table.set_string_descriptor(desc, String_descriptor_zero::LANGID::ENUS, 2);
	}
	{
		String_descriptor_base desc;
		desc.assign(usb_id_str.data());
		usb_desc_table.set_string_descriptor(desc, String_descriptor_zero::LANGID::ENUS, 3);
	}
	{
		String_descriptor_base desc;
		desc.assign("Default configuration");
		usb_desc_table.set_string_descriptor(desc, String_descriptor_zero::LANGID::ENUS, 4);
	}
	{
		String_descriptor_base desc;
		desc.assign("Communications");
		usb_desc_table.set_string_descriptor(desc, String_descriptor_zero::LANGID::ENUS, 5);
	}
	{
		String_descriptor_base desc;
		desc.assign("CDC Data");
		usb_desc_table.set_string_descriptor(desc, String_descriptor_zero::LANGID::ENUS, 6);
	}
	std::shared_ptr<CDC::CDC_header_descriptor> cdc_header_desc = std::make_shared<CDC::CDC_header_descriptor>();
	cdc_header_desc->bcdCDC = USB_common::build_bcd(1,1,0);
	usb_desc_table.add_other_descriptor(cdc_header_desc);

	std::shared_ptr<CDC::CDC_call_management_descriptor> cdc_call_mgmt_desc = std::make_shared<CDC::CDC_call_management_descriptor>();
	// cdc_call_mgmt_desc->bmCapabilities = 0x00;
	cdc_call_mgmt_desc->set_call_mgmt(CDC::CDC_call_management_descriptor::CALL_MGMT_CHANNEL::COMM_CLASS_IFACE);
	cdc_call_mgmt_desc->set_self_call_mgmt_handle(false);
	cdc_call_mgmt_desc->bDataInterface = 1;
	usb_desc_table.add_other_descriptor(cdc_call_mgmt_desc);

	std::shared_ptr<CDC::CDC_acm_descriptor> cdc_acm_desc = std::make_shared<CDC::CDC_acm_descriptor>();
	// cdc_acm_desc->bmCapabilities = 0x00;
	cdc_acm_desc->set_support_network_connection(true);
	cdc_acm_desc->set_support_send_break(false);
	cdc_acm_desc->set_support_line(true);
	cdc_acm_desc->set_support_comm(true);
	// cdc_acm_desc->bmCapabilities = 0x03;
	usb_desc_table.add_other_descriptor(cdc_acm_desc);

	std::shared_ptr<CDC::CDC_union_descriptor> cdc_union_desc = std::make_shared<CDC::CDC_union_descriptor>();
	cdc_union_desc->bMasterInterface = 0;
	cdc_union_desc->bSlaveInterface0 = 1;
	usb_desc_table.add_other_descriptor(cdc_union_desc);
	
	Config_desc_table::Config_desc_ptr config_desc_ptr = usb_desc_table.get_config_descriptor(0);

	//register iface and ep to configuration
	config_desc_ptr->get_desc_list().push_back( usb_desc_table.get_interface_descriptor(0).get() );
	config_desc_ptr->get_desc_list().push_back( cdc_header_desc.get() );
	config_desc_ptr->get_desc_list().push_back( cdc_acm_desc.get() );
	config_desc_ptr->get_desc_list().push_back( cdc_union_desc.get() );
	config_desc_ptr->get_desc_list().push_back( cdc_call_mgmt_desc.get() );
	config_desc_ptr->get_desc_list().push_back( usb_desc_table.get_endpoint_descriptor(0x82).get() );

	config_desc_ptr->get_desc_list().push_back( usb_desc_table.get_interface_descriptor(1).get() );
	config_desc_ptr->get_desc_list().push_back( usb_desc_table.get_endpoint_descriptor(0x01).get() );
	config_desc_ptr->get_desc_list().push_back( usb_desc_table.get_endpoint_descriptor(0x81).get() );

	config_desc_ptr->wTotalLength = config_desc_ptr->get_total_size();

	m_rx_buf.resize(1024);
	m_rx_buf_adapter.reset(m_rx_buf.data(), m_rx_buf.size());

	m_tx_buf.resize(1024);
	m_tx_buf_adapter.reset(m_tx_buf.data(), m_tx_buf.size());

	logger->log(LOG_LEVEL::INFO, "main", "usb_core.initialize");
	if(!usb_core.initialize(&usb_driver, 8, m_tx_buf_adapter, m_rx_buf_adapter))
	{
		return false;
	}

	usb_core.set_usb_class(&usb_cdc);
	usb_core.set_descriptor_table(&usb_desc_table);
	usb_core.set_config_callback(&handle_usb_set_config_thunk, this);

	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	/**USB_OTG_HS GPIO Configuration    
	PC0     ------> USB_OTG_HS_ULPI_STP
	PC2_C     ------> USB_OTG_HS_ULPI_DIR
	PC3_C     ------> USB_OTG_HS_ULPI_NXT
	PA3     ------> USB_OTG_HS_ULPI_D0
	PA5     ------> USB_OTG_HS_ULPI_CK
	PB0     ------> USB_OTG_HS_ULPI_D1
	PB1     ------> USB_OTG_HS_ULPI_D2
	PB10     ------> USB_OTG_HS_ULPI_D3
	PB11     ------> USB_OTG_HS_ULPI_D4
	PB12     ------> USB_OTG_HS_ULPI_D5
	PB13     ------> USB_OTG_HS_ULPI_D6
	PB5     ------> USB_OTG_HS_ULPI_D7 
	*/
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_2|GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF10_OTG2_HS;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_5;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF10_OTG2_HS;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_10|GPIO_PIN_11 
	                      |GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_5;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF10_OTG2_HS;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	__HAL_RCC_USB_OTG_HS_CLK_ENABLE();
	__HAL_RCC_USB_OTG_HS_ULPI_CLK_ENABLE();

	// TODO: switch to isr mode
	// HAL_NVIC_SetPriority(OTG_HS_IRQn, 5, 0);
	// HAL_NVIC_EnableIRQ(OTG_HS_IRQn);

	// usb_core.set_control_callback(std::bind(&Main_task::usb_control_callback, this, std::placeholders::_1));
	// usb_core.set_config_callback(std::bind(&Main_task::usb_config_callback, this));
	// usb_core.set_descriptor_callback(std::bind(&Main_task::usb_get_descriptor_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	logger->log(LOG_LEVEL::INFO, "main", "usb_core.enable");
	if(!usb_core.enable())
	{
		logger->log(LOG_LEVEL::INFO, "main", "usb_core.enable failed");
		return false;
	}

	logger->log(LOG_LEVEL::INFO, "main", "usb_core.connect");
	if(!usb_core.connect())
	{
		logger->log(LOG_LEVEL::INFO, "main", "usb_core.connect failed");
		return false;
	}

	return true;
}

bool Main_task::mount_fs()
{
	freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();

	logger->log(LOG_LEVEL::INFO, "main", "Ready");

	{
		std::lock_guard<Mutex_static_recursive> lock(can_usb_app.get_mutex());

		W25Q16JV& m_qspi = can_usb_app.get_flash();
		W25Q16JV_conf_region& m_fs = can_usb_app.get_fs();

		m_qspi.set_handle(&hqspi);

		if(!m_qspi.init())
		{
			logger->log(LOG_LEVEL::ERROR, "main", "m_qspi.init failed");

			for(;;)
			{
				vTaskSuspend(nullptr);
			}
		}

		m_fs.initialize();
		m_fs.set_flash(&m_qspi);

		uint8_t mfg_id = 0;
		uint16_t flash_pn = 0;
		if(m_qspi.get_jdec_id(&mfg_id, &flash_pn))
		{
			logger->log(LOG_LEVEL::INFO, "main", "flash mfg id %02" PRIX32, uint32_t(mfg_id));
			logger->log(LOG_LEVEL::INFO, "main", "flash pn %04" PRIX32, uint32_t(flash_pn));
		}
		else
		{
			logger->log(LOG_LEVEL::ERROR, "main", "get_jdec_id failed");
		}

		uint64_t unique_id = 0;
		if(m_qspi.get_unique_id(&unique_id))
		{
			// logger->log(LOG_LEVEL::INFO, "main", "flash sn %016" PRIX64, unique_id);
			//aparently PRIX64 is broken
			logger->log(LOG_LEVEL::INFO, "main", "flash sn %08" PRIX32 "%08" PRIX32, Byte_util::get_upper_half(unique_id), Byte_util::get_lower_half(unique_id));
		}
		else
		{
			logger->log(LOG_LEVEL::ERROR, "main", "get_unique_id failed");
		}

		logger->log(LOG_LEVEL::INFO, "main", "Mounting flash fs");
		int mount_ret = m_fs.mount();
		if(mount_ret != SPIFFS_OK)
		{
			logger->log(LOG_LEVEL::ERROR, "main", "Flash mount failed: %d", mount_ret);
			logger->log(LOG_LEVEL::ERROR, "main", "You will need to reload the config");

			logger->log(LOG_LEVEL::INFO, "main", "Format flash");
			int format_ret = m_fs.format();
			if(format_ret != SPIFFS_OK)
			{
				logger->log(LOG_LEVEL::FATAL, "main", "Flash format failed: %d", format_ret);
				logger->log(LOG_LEVEL::FATAL, "main", "Try a power cycle, your board may be broken");

				for(;;)
				{
					vTaskSuspend(nullptr);
				}
			}

			logger->log(LOG_LEVEL::INFO, "main", "Mounting flash fs");
			mount_ret = m_fs.mount();
			if(mount_ret != SPIFFS_OK)
			{
				logger->log(LOG_LEVEL::FATAL, "main", "Flash mount failed right after we formatted it: %d", mount_ret);
				logger->log(LOG_LEVEL::FATAL, "main", "Try a power cycle, your board may be broken");

				for(;;)
				{
					vTaskSuspend(nullptr);
				}
			}
			else
			{
				logger->log(LOG_LEVEL::INFO, "main", "Flash mount ok");
			}

			//write default config
			if(!can_usb_app.write_default_config())
			{
				logger->log(LOG_LEVEL::FATAL, "main", "Writing default config failed");

				for(;;)
				{
					vTaskSuspend(nullptr);
				}
			}
			if(!can_usb_app.write_default_bitrate_table())
			{
				logger->log(LOG_LEVEL::FATAL, "main", "Writing default bitrate table failed");

				for(;;)
				{
					vTaskSuspend(nullptr);
				}
			}
		}
		else
		{
			logger->log(LOG_LEVEL::INFO, "main", "Flash mount ok");
		}
	}

	return true;
}

bool Main_task::load_config()
{
	freertos_util::logging::Logger* const logger = freertos_util::logging::Global_logger::get();

	logger->log(LOG_LEVEL::INFO, "main", "Load config");
	if(!can_usb_app.load_config())
	{
		logger->log(LOG_LEVEL::WARN, "main", "Config load failed, restoring default");

		if(!can_usb_app.write_default_config())
		{
			logger->log(LOG_LEVEL::FATAL, "main", "Writing default config load failed");

			for(;;)
			{
				vTaskSuspend(nullptr);
			}
		}
		else
		{
			logger->log(LOG_LEVEL::WARN, "main", "Default config wrote ok");
		}
	}
	else
	{
		logger->log(LOG_LEVEL::INFO, "main", "Config ok");
	}

	if(!can_usb_app.load_bitrate_table())
	{
		logger->log(LOG_LEVEL::WARN, "main", "Bitrate table load failed, restoring default");

		if(!can_usb_app.write_default_bitrate_table())
		{
			logger->log(LOG_LEVEL::FATAL, "main", "Writing default bitrate table failed");

			for(;;)
			{
				vTaskSuspend(nullptr);
			}
		}
		else
		{
			logger->log(LOG_LEVEL::WARN, "main", "Default bitrate table wrote ok");
		}
	}
	else
	{
		logger->log(LOG_LEVEL::INFO, "main", "Bitrate table ok");
	}

	return true;
}

bool Main_task::handle_usb_set_config_thunk(void* ctx, const uint16_t config)
{
	return static_cast<Main_task*>(ctx)->handle_usb_set_config(config);
}

bool Main_task::handle_usb_set_config(const uint8_t config)
{
	bool ret = false;


	switch(config)
	{
		case 0:
		{
			usb_core.get_driver()->ep_stall(0x01);
			usb_core.get_driver()->ep_stall(0x81);
			usb_core.get_driver()->ep_stall(0x82);
			ret = true;
			break;
		}
		case 1:
		{
			// Config_desc_table::Config_desc_const_ptr config_ptr = usb_desc_table.get_config_descriptor(config);
			
			// Iface_desc_table::Iface_desc_const_ptr ctrl_iface_ptr = usb_desc_table.get_interface_descriptor(0);
			// Iface_desc_table::Iface_desc_const_ptr data_iface_ptr = usb_desc_table.get_interface_descriptor(1);

			//out 1
			{
				Endpoint_desc_table::Endpoint_desc_const_ptr ep_data_out  = usb_desc_table.get_endpoint_descriptor(0x01);

				usb_driver_base::ep_cfg ep1;
				ep1.num  = ep_data_out->bEndpointAddress;
				ep1.size = ep_data_out->wMaxPacketSize;
				ep1.type = usb_driver_base::EP_TYPE::BULK;
				usb_core.get_driver()->ep_config(ep1);
			}
			//in 1
			{
				Endpoint_desc_table::Endpoint_desc_const_ptr ep_data_in   = usb_desc_table.get_endpoint_descriptor(0x81);
				
				usb_driver_base::ep_cfg ep2;
				ep2.num  = ep_data_in->bEndpointAddress;
				ep2.size = ep_data_in->wMaxPacketSize;
				ep2.type = usb_driver_base::EP_TYPE::BULK;
				usb_core.get_driver()->ep_config(ep2);
			}
			//in 2
			{
				Endpoint_desc_table::Endpoint_desc_const_ptr ep_notify_in = usb_desc_table.get_endpoint_descriptor(0x82);

				usb_driver_base::ep_cfg ep3;
				ep3.num  = ep_notify_in->bEndpointAddress;
				ep3.size = ep_notify_in->wMaxPacketSize;
				ep3.type = usb_driver_base::EP_TYPE::INTERRUPT;
				usb_core.get_driver()->ep_config(ep3);
			}

			test_usb_cdc.notify_new_connection();

			ret = true;
			break;
		}
		default:
		{
			ret = false;
			break;
		}
	}

	return ret;
}
