#pragma once

#include "freertos_cpp_util/Task_static.hpp"

#include "CAN_USB_app.hpp"

#include "libusb_dev_cpp/core/usb_core.hpp"
#include "libusb_dev_cpp/class/cdc/cdc_usb.hpp"
#include "libusb_dev_cpp/driver/stm32/stm32_h7xx_otghs2.hpp"
#include "libusb_dev_cpp/util/Descriptor_table.hpp"
#include "libusb_dev_cpp/util/EP_buffer_mgr_freertos.hpp"

class Main_task : public Task_static<2048>
{
public:
	Main_task();

	void work() override;

	bool init_usb();

	bool mount_fs();

	bool load_config();

	static bool handle_usb_set_config_thunk(void* ctx, const uint16_t config);
	static void handle_usb_reset_thunk(void* ctx);

protected:
	
	bool handle_usb_set_config(const uint8_t config);
	void handle_usb_reset();

	void get_unique_id(std::array<uint32_t, 3>* id);
	void get_unique_id_str(std::array<char, 25>* id_str);

	static void sw_reset();
	static void jump_to_addr(uint32_t estack, uint32_t jump_addr);

	Descriptor_table usb_desc_table;
	std::array<char, 25> usb_id_str;//this is read by the usb core, and sent as a descriptor payload
	std::vector<uint8_t> m_rx_buf;
	Buffer_adapter_rx m_rx_buf_adapter;
	std::vector<uint8_t> m_tx_buf;
	Buffer_adapter_tx m_tx_buf_adapter;

	//only enumerate once, if told to reset again cause an actual sw reset
	bool has_enumerated;
};

extern USB_core         usb_core;
extern CDC_class        usb_cdc;
extern stm32_h7xx_otghs2 usb_driver;
extern EP_buffer_mgr_freertos<1, 8, 64,  32> usb_ep0_buffer;
extern EP_buffer_mgr_freertos<3, 4, 512, 32> usb_tx_buffer;
extern EP_buffer_mgr_freertos<3, 4, 512, 32> usb_rx_buffer;