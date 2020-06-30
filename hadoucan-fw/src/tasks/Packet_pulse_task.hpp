#pragma once

#include "freertos_cpp_util/Task_static.hpp"

class Packet_pulse_task : public Task_static<1024>
{
public:

	Packet_pulse_task();

	~Packet_pulse_task() override;

	void work() override;

	void bitbang_50us_pulse();

	void microsleep(const uint32_t val);

	unsigned int m_local_packet_counter;
};
