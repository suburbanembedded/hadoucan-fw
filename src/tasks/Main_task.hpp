#pragma once

class Main_task : public Task_static<512>
{
public:
	void work() override
	{
		//init
		usb_rx_buffer_task.set_usb_rx(&usb_rx_task);
		usb_tx_buffer_task.set_usb_tx(&usb_tx_task);
		usb_lawicel_task.set_usb_tx(&usb_tx_buffer_task);
		usb_lawicel_task.set_usb_rx(&usb_rx_buffer_task);

		//TODO: refactor can handle init
		hfdcan1.Instance = FDCAN1;
		stm32_fdcan_rx_task.set_packet_callback(&can_rx_to_lawicel);
		stm32_fdcan_rx_task.set_can_instance(FDCAN1);
		stm32_fdcan_rx_task.set_can_handle(&hfdcan1);

		//can RX
		stm32_fdcan_rx_task.launch("stm32_fdcan_rx", 1);

		//protocol state machine
		usb_lawicel_task.launch("usb_lawicel", 2);

		//process usb packets
		usb_rx_buffer_task.launch("usb_rx_buf", 4);
		usb_tx_buffer_task.launch("usb_tx_buf", 5);

		//actually send usb packets on the wire
		usb_rx_task.launch("usb_rx", 3);
		usb_tx_task.launch("usb_tx", 4);

		led_task.launch("led", 1);
		qspi_task.launch("qspi", 1);
		timesync_task.launch("timesync", 1);

		uart1_log<64>(LOG_LEVEL::INFO, "main", "Ready");

		for(;;)
		{
			vTaskSuspend(nullptr);
		}
	}
};