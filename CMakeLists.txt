cmake_minimum_required(VERSION 3.5)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR arm-none-eabi)

project(CANUSBFDISO C CXX ASM)

set(ATOLLIC_PATH "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.2.0/ARMTools/bin")

set(CMAKE_ASM_COMPILER ${ATOLLIC_PATH}/arm-atollic-eabi-gcc)
set(CMAKE_C_COMPILER   ${ATOLLIC_PATH}/arm-atollic-eabi-gcc)
set(CMAKE_CXX_COMPILER ${ATOLLIC_PATH}/arm-atollic-eabi-g++)

set(CXX_STANDARD 11)
set(C_STANDARD 11)

set(INSTALL_GTEST OFF)

add_definitions(
	-DSTM32H750xx
	-D__weak=__attribute__\(\(weak\)\)
	-D__packed=__attribute__\(\(__packed__\)\)
	-DUSE_HAL_DRIVER
	-DWORD_SIZE=4
	-DGTEST_HAS_PTHREAD=0
	-DGTEST_HAS_CLONE=0
	-DGTEST_HAS_POSIX_RE=0
	-DGTEST_LINKED_AS_SHARED_LIBRARY=0
	-DGTEST_CREATE_SHARED_LIBRARY=0
)

set(CMAKE_ASM_FLAGS "-mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Wa,--warn -x assembler-with-cpp")
set(CMAKE_ASM_FLAGS_DEBUG "-O0 -ggdb")
set(CMAKE_ASM_FLAGS_RELEASE "-Os")

set(CMAKE_C_FLAGS "-mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-sp-d16 -ffunction-sections -fdata-sections -fstack-usage -Wall -specs=nano.specs -Werror=return-type")
set(CMAKE_C_FLAGS_DEBUG "-O0 -ggdb")
set(CMAKE_C_FLAGS_RELEASE "-Os")

set(CMAKE_CXX_FLAGS "-mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-sp-d16 -ffunction-sections -fdata-sections -fstack-usage  -fno-threadsafe-statics -Wall -specs=nano.specs -Werror=return-type")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-Os")

set(CMAKE_EXE_LINKER_FLAGS "-mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-sp-d16 -T\"${CMAKE_CURRENT_SOURCE_DIR}/STM32H750VB_FLASH.ld\" -specs=nosys.specs -static -Wl,-cref,-u,Reset_Handler \"-Wl,-Map=canusbfdiso.map\" -Wl,--gc-sections -Wl,--defsym=malloc_getpagesize_P=0x80")

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_subdirectory(external/stm32cube_fw_h7_v1_3_0)
add_subdirectory(external/freertos_cpp_util)
add_subdirectory(external/common_util)
#add_subdirectory(external/googletest)

add_library(gtest_stm32
	external/googletest/googletest/src/gtest-all.cc
)
target_include_directories(gtest_stm32
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/external/googletest/googletest/include
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/external/googletest/googletest
)

add_library(gmock_stm32
	external/googletest/googlemock/src/gmock-all.cc
)
target_include_directories(gmock_stm32
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/external/googletest/googlemock/include
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/external/googletest/googlemock
)

target_link_libraries(gmock_stm32
	gtest_stm32
)

add_executable(canusbfdiso
	src/stm32h7xx_it.c
	src/stm32h7xx_hal_timebase_tim.c
	src/stm32h7xx_hal_msp.c

	src/syscalls.c
	
	src/main.cpp
	src/main_cube.c
	src/hal_inst.c

	src/USB_RX_task.cpp
	src/USB_TX_task.cpp
	src/USB_buf.cpp
	
	src/USB_tx_buffer_task.cpp
	src/USB_rx_buffer_task.cpp

	src/uart1_printf.cpp

	src/freertos_stubs.cpp
	src/freertos_new.cpp
	src/newlib_stubs.c

	src/usb/usbd_conf.c
	src/usb/usbd_desc.c
	src/usb/usb_device.c

	#src/msg/acn.c
	#src/msg/asn1crt.c
	#src/msg/ber.c
	#src/msg/can_packet.c
	#src/msg/real.c
	#src/msg/sm-1301_rpc.c
	#src/msg/xer.c

	src/lawicel/can_dlc.cpp
	src/lawicel/Lawicel_parser.cpp
	src/lawicel/Lawicel_parser_stm32.cpp

	src/STM32_fdcan_tx.cpp
	src/STM32_fdcan_rx.cpp
)

target_link_libraries(canusbfdiso

	stm32h7xx_hal
	stm32h7xx_ll
	stm32h750xx_cmsis

	stm32_usb_lib_core
	stm32_usb_lib_cdc

	freertos_v9_heap4
	freertos_v9
	freertos_v9_cmsis

	common_util
	freertos_cpp_util
)

#target_link_libraries(canusbfdiso_tests
#
#	stm32h7xx_hal
#	stm32h7xx_ll
#	stm32h750xx_cmsis
#
#	stm32_usb_lib_core
#	stm32_usb_lib_cdc
#
#	freertos_v9_heap4
#	freertos_v9
#	freertos_v9_cmsis
#
#	freertos_cpp_util
#
#	gtest_stm32
#	gmock_stm32
#)

# Generate intel hex image
add_custom_command(
	OUTPUT canusbfdiso.hex
	COMMAND ${ATOLLIC_PATH}/arm-atollic-eabi-objcopy -Oihex canusbfdiso canusbfdiso.hex
	DEPENDS canusbfdiso
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	VERBATIM	
)

add_custom_target(canusbfdiso_hex
	ALL
	DEPENDS canusbfdiso.hex
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Print the image size
add_custom_target(canusbfdiso_size
	ALL
	COMMAND ${ATOLLIC_PATH}/arm-atollic-eabi-size --format=sysv canusbfdiso
	DEPENDS canusbfdiso
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	VERBATIM
)

add_dependencies(canusbfdiso_size
	canusbfdiso
)
