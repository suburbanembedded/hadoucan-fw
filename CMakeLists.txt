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

add_definitions(
	-DSTM32H750xx
	-D__weak=__attribute__\(\(weak\)\)
	-D__packed=__attribute__\(\(__packed__\)\)
	-DUSE_HAL_DRIVER
)

set(CMAKE_ASM_FLAGS "-mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Wa,--warn -x assembler-with-cpp")
set(CMAKE_ASM_FLAGS_DEBUG "-O0 -ggdb")
set(CMAKE_ASM_FLAGS_RELEASE "-Os")

set(CMAKE_C_FLAGS "-mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-sp-d16 -ffunction-sections -fdata-sections -fstack-usage -Wall")
set(CMAKE_C_FLAGS_DEBUG "-O0 -ggdb")
set(CMAKE_C_FLAGS_RELEASE "-Os")

set(CMAKE_CXX_FLAGS "-mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-sp-d16 -ffunction-sections -fdata-sections -fstack-usage -fno-rtti -fno-exceptions -fno-threadsafe-statics -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-Os")

set(CMAKE_EXE_LINKER_FLAGS "-mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-sp-d16 -T\"../STM32H750VB_FLASH.ld\" -specs=nosys.specs -static -Wl,-cref,-u,Reset_Handler \"-Wl,-Map=canusbfdiso.map\" -Wl,--gc-sections -Wl,--defsym=malloc_getpagesize_P=0x1000")

#set(CMAKE_EXE_LINKER_FLAGS_INIT -T\"../STM32H750VB_FLASH.ld\" -specs=nosys.specs -static -Wl,-cref,-u,Reset_Handler \"-Wl,-Map=canusbfdiso.map\" -Wl,--gc-sections -Wl,--defsym=malloc_getpagesize_P=0x1000 -specs=nano.specs)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_subdirectory(stm32cube_fw_h7_v1_3_0)

add_executable(canusbfdiso
	#src/startup_stm32h750xx.s
	src/syscalls.c
	#src/system_stm32h7xx.c
	src/tiny_printf.c
	src/stm32h7xx_it.c
	src/stm32h7xx_hal_timebase_tim.c

	src/main.cpp
	src/FreeRTOS_static_task.cpp
	src/FreeRTOS_task_base.cpp
	src/FreeRTOS_task.cpp
)

target_link_libraries(canusbfdiso
	c
	m
	stdc++
	supc++

	stm32h7xx_hal
	stm32h7xx_ll
	stm32h750xx_cmsis

	freertos_v9
	freertos_v9_heap4
	freertos_v9_cmsis
)

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
