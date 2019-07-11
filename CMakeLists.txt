cmake_minimum_required(VERSION 3.5)

project(canusbfdiso C CXX ASM)

set(INSTALL_GTEST OFF)

add_definitions(
	-DSTM32H750xx
	-D__weak=__attribute__\(\(weak\)\)
	-D__packed=__attribute__\(\(__packed__\)\)

	-DUSE_HAL_DRIVER
	-DHSE_VALUE=\(\(uint32_t\)24000000\)
	
	-DWORD_SIZE=4

	#-DGTEST_HAS_PTHREAD=0
	#-DGTEST_HAS_CLONE=0
	#-DGTEST_HAS_POSIX_RE=0
	#-DGTEST_LINKED_AS_SHARED_LIBRARY=0
	#-DGTEST_CREATE_SHARED_LIBRARY=0

	-DSPIFFS_USE_MAGIC
	-DSPIFFS_USE_MAGIC_LENGTH
	-DSPIFFS_LOCK=spiffs_lock_dispatch
	-DSPIFFS_UNLOCK=spiffs_unlock_dispatch
	-DSPIFFS_HAL_CALLBACK_EXTRA
	-DSPIFFS_ALIGNED_OBJECT_INDEX_TABLES=1

	#we will provide the syscalls
	#-D_REENT_ONLY
	#-DREENTRANT_SYSCALLS_PROVIDED

	-DMBEDTLS_CONFIG_FILE=<config-no-entropy.h>
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/src
	external/mbedtls/include
)

add_library(spiffs
	external/spiffs/src/spiffs_cache.c
	external/spiffs/src/spiffs_check.c
	external/spiffs/src/spiffs_gc.c
	external/spiffs/src/spiffs_hydrogen.c
	external/spiffs/src/spiffs_nucleus.c
)

target_include_directories(spiffs PUBLIC
	external/spiffs/src
)

#add_subdirectory(external/stm32cube_fw_h7_v1_3_0)
add_subdirectory(external/stm32cube_fw_h7_v1_4_0)
add_subdirectory(external/tinyxml2)
add_subdirectory(external/mbedtls)
add_subdirectory(external/crc)

add_subdirectory(external/bootloader_util)
add_subdirectory(external/common_util)
add_subdirectory(external/emb_fs_util)
add_subdirectory(external/freertos_cpp_util)
add_subdirectory(external/libusb_dev_cpp)
add_subdirectory(external/mbedtls_util)
add_subdirectory(external/tinyxml2_util)

add_executable(canusbfdiso.elf
	src/system_stm32h7xx.c

	src/stm32h7xx_it.c
	src/stm32h7xx_hal_timebase_tim.c
	src/stm32h7xx_hal_msp.c

	#src/syscalls.c
	
	src/main.cpp
	src/main_cube.c
	src/hal_inst.c

	src/global_app_inst.cpp

	
	src/USB_buf.cpp
	src/USB_tx_buffer_task.cpp
	src/USB_rx_buffer_task.cpp

	src/uart1_printf.cpp

	src/freertos_stubs.cpp
	src/freertos_new.cpp
	src/newlib_stubs.c

	src/lawicel/CAN_DLC.cpp
	src/lawicel/STM32_FDCAN_DLC.cpp
	src/lawicel/Lawicel_parser.cpp
	src/lawicel/Lawicel_parser_stm32.cpp

	src/STM32_fdcan_tx.cpp

	src/tasks/STM32_fdcan_rx.cpp
	src/tasks/LED_task.cpp
	src/tasks/Timesync_task.cpp
	src/tasks/USB_lawicel_task.cpp
	src/tasks/Main_task.cpp
	src/tasks/Task_instances.cpp

	src/CAN_USB_app.cpp
	src/CAN_USB_app_config.cpp
	src/CAN_USB_app_bitrate_table.cpp
)

target_link_libraries(canusbfdiso.elf

	stm32h7xx_hal
	stm32h7xx_ll
	stm32h750xx_cmsis

	freertos_v10_heap4
	freertos_v10
	freertos_v10_cmsis

	bootloader_util
	common_util
	freertos_cpp_util
	usb_dev_cpp
	mbedtls_util
	tinyxml2_util

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
	OUTPUT canusbfdiso.hex
	COMMAND ${ATOLLIC_PATH}/arm-atollic-eabi-objcopy -S -Oihex canusbfdiso.elf canusbfdiso.hex
	DEPENDS canusbfdiso.elf
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	VERBATIM	
)

add_custom_target(canusbfdiso_hex
	ALL
	DEPENDS canusbfdiso.hex
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Generate raw image
add_custom_command(
	OUTPUT canusbfdiso.bin
	COMMAND ${ATOLLIC_PATH}/arm-atollic-eabi-objcopy -S -Obinary canusbfdiso.elf canusbfdiso.bin
	DEPENDS canusbfdiso.elf
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	VERBATIM	
)

add_custom_target(canusbfdiso_bin
	ALL
	DEPENDS canusbfdiso.bin
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Print the image size
add_custom_target(canusbfdiso_size
	ALL
	COMMAND ${ATOLLIC_PATH}/arm-atollic-eabi-size --format=sysv canusbfdiso.elf
	DEPENDS canusbfdiso.elf
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	VERBATIM
)

add_custom_target(upload_canusbfdiso
	COMMAND openocd -f /usr/local/share/openocd/scripts/interface/stlink.cfg -f /usr/local/share/openocd/scripts/target/stm32h7x.cfg -c "init;reset halt;load_image canusbfdiso.elf;verify_image canusbfdiso.elf;mem2array estack_reset_handler 32 0x24000000 2;reg msp $estack_reset_handler(0); resume $estack_reset_handler(1);exit"
	DEPENDS canusbfdiso.elf
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	VERBATIM
)