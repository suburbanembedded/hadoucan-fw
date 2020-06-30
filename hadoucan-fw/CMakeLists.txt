
add_executable(hadoucan-fw.elf
	src/system_stm32h7xx.c

	src/stm32h7xx_it.c
	src/stm32h7xx_hal_timebase_tim.c
	src/stm32h7xx_hal_msp.c

	#src/syscalls.c
	
	src/main.cpp
	src/main_cube.c
	src/hal_inst.c
	src/sw_ver.cpp

	src/global_app_inst.cpp

	src/tasks/USB_tx_buffer_task.cpp
	src/tasks/USB_rx_buffer_task.cpp

	src/freertos_stubs.cpp
	src/freertos_new.cpp
	src/newlib_stubs.c

	src/lawicel/CAN_DLC.cpp
	src/lawicel/STM32_FDCAN_DLC.cpp
	src/lawicel/Lawicel_parser.cpp
	src/lawicel/Lawicel_parser_stm32.cpp

	src/STM32_fdcan_tx.cpp

	src/tasks/LED_task.cpp
	src/tasks/Logging_task.cpp
	src/tasks/Main_task.cpp
	src/tasks/STM32_fdcan_rx.cpp
	src/tasks/Task_instances.cpp
	src/tasks/Timesync_task.cpp
	src/tasks/USB_lawicel_task.cpp
	src/tasks/USB_poll.cpp
	src/tasks/System_mon_task.cpp
	src/tasks/Packet_pulse_task.cpp

	src/CAN_USB_app.cpp
	src/CAN_USB_app_config.cpp
	src/CAN_USB_app_bitrate_table.cpp
)

target_compile_definitions(hadoucan-fw.elf
	PRIVATE
		-DGIT_SHA1="${GIT_SHA1}"
)

target_include_directories(hadoucan-fw.elf PUBLIC
	include
	src
)

target_link_libraries(hadoucan-fw.elf

	stm32h7xx_hal
	stm32h7xx_ll
	stm32h750xx_cmsis

	freertos_v10_heap4
	freertos_v10
	freertos_v10_cmsis

	bootloader_util
	common_util
	freertos_cpp_util
	mbedtls_util
	tinyxml2_util
	
	usb_dev_cpp
	usb_dev_cpp_stm32

	spi_flash
	spiffs_flash
	spiffs_util

	crc

	tinyxml2_static

	mbedtls
	mbedcrypto
)

# Generate intel hex image
add_custom_command(
	OUTPUT hadoucan-fw.hex
	COMMAND ${ATOLLIC_PATH}/arm-atollic-eabi-objcopy -S -Oihex hadoucan-fw.elf hadoucan-fw.hex
	DEPENDS hadoucan-fw.elf
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	VERBATIM	
)

add_custom_target(hadoucan-fw_hex
	ALL
	DEPENDS hadoucan-fw.hex
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Generate raw image
add_custom_command(
	OUTPUT hadoucan-fw.bin
	COMMAND ${ATOLLIC_PATH}/arm-atollic-eabi-objcopy -S -Obinary hadoucan-fw.elf hadoucan-fw.bin
	DEPENDS hadoucan-fw.elf
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	VERBATIM	
)

add_custom_target(hadoucan-fw_bin
	ALL
	DEPENDS hadoucan-fw.bin
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Print the image size
add_custom_target(hadoucan-fw_size
	ALL
	COMMAND ${ATOLLIC_PATH}/arm-atollic-eabi-size --format=sysv hadoucan-fw.elf
	DEPENDS hadoucan-fw.elf
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	VERBATIM
)

add_custom_target(upload_hadoucan-fw
	COMMAND openocd -f /usr/local/share/openocd/scripts/interface/stlink.cfg -f /usr/local/share/openocd/scripts/target/stm32h7x.cfg -c "init;reset halt;load_image hadoucan-fw.elf;verify_image hadoucan-fw.elf;mem2array estack_reset_handler 32 0x24000000 2;reg msp $estack_reset_handler(0); resume $estack_reset_handler(1);exit"
	DEPENDS hadoucan-fw.elf
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	VERBATIM
)

if(DEFINED Doxygen::doxygen)
	doxygen_add_docs(hadoucan-fw_docs
		include/
		src/
	)
endif()
