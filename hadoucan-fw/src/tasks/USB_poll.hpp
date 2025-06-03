#pragma once

#include "freertos_cpp_util/Task_static.hpp"
#include "freertos_cpp_util/Event_group_static.hpp"

#include "tusb.h"

#include <atomic>
#include <array>

class USB_core_task : public Task_static<2048>
{
	friend void tud_suspend_cb(bool remote_wakeup_en);
	friend void tud_resume_cb(void);
	friend void tud_mount_cb(void);
	friend void tud_umount_cb(void);
	friend void tud_cdc_line_state_cb(uint8_t instance, bool dtr, bool rts);
	friend void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const* p_line_coding);
	friend void tud_cdc_rx_cb(uint8_t itf);
	friend void tud_cdc_tx_complete_cb(uint8_t itf);

public:

	USB_core_task() : m_dtr(false), m_rts(false), m_bit_rate(0), m_stop_bits(0), m_parity(0), m_data_bits(0)
	{

	}

	void work() override;

	std::atomic<bool> m_dtr;
	std::atomic<bool> m_rts;

	std::atomic<unsigned> m_bit_rate;
	std::atomic<unsigned> m_stop_bits;
	std::atomic<unsigned> m_parity;
	std::atomic<unsigned> m_data_bits;

	void wait_for_usb_rx_avail();
	void wait_for_usb_tx_complete();
	
	static void get_unique_id(std::array<uint32_t, 3>* const id);
	static void get_unique_id_str(std::array<char, 25>* const id_str);

private:
	const static EventBits_t RX_AVAIL_BIT = 0x0001U;
	const static EventBits_t TX_COMPL_BIT = 0x0002U;
	Event_group_static m_events;
};
