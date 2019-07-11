#pragma once

#include "freertos_cpp_util/Task_static.hpp"

#include "CAN_USB_app.hpp"

#include "libusb_dev_cpp/usb_core.hpp"
#include "libusb_dev_cpp/driver/stm32/stm32_h7xx_otghs.hpp"
#include "libusb_dev_cpp/util/Descriptor_table.hpp"

class Main_task : public Task_static<4096>
{
public:
	void work() override;

	bool init_usb();

	bool mount_fs();

	bool load_config();

protected:
	
	void get_unique_id(std::array<uint32_t, 3>* id);
	void get_unique_id_str(std::array<char, 25>* id_str);

	Descriptor_table usb_desc_table;
	std::array<char, 25> usb_id_str;//this is read by the usb core, and sent as a descriptor payload
};


extern USB_core         usb_core;
extern stm32_h7xx_otghs usb_driver;
extern EP_buffer_mgr_freertos<3, 4, 512, 32> usb_tx_buffer;
extern EP_buffer_mgr_freertos<3, 4, 512, 32> usb_rx_buffer;