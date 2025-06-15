#pragma once

#include "freertos_cpp_util/Task_static.hpp"

#include "CAN_USB_app.hpp"

class Main_task : public Task_static<2048>
{
public:
	void work() override;

	bool init_usb();

	bool mount_fs();

	bool load_config();

protected:
	
	void get_unique_id(std::array<uint32_t, 3>* id);
	void get_unique_id_str(std::array<char, 25>* id_str);

	std::array<char, 25> usb_id_str;//this is read by the usb core, and sent as a descriptor payload
};
