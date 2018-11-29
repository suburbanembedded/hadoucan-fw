cmake_minimum_required(VERSION 3.5)

project(CANUSBFDISO C CXX ASM)

#set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
#set(CMAKE_C_COMPILER arm-none-eabi-gcc)
#set(CMAKE_CXX_COMPILER arm-none-eabi-g++)

set(CMAKE_ASM_COMPILER /opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.1.0/ARMTools/bin/arm-atollic-eabi-gcc)
set(CMAKE_C_COMPILER /opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.1.0/ARMTools/bin/arm-atollic-eabi-gcc)
set(CMAKE_CXX_COMPILER /opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.1.0/ARMTools/bin/arm-atollic-eabi-g++)

set(CXX_STANDARD 11)
set(C_STANDARD 99)

add_definitions(-DSTM32H750xx)

set(CMAKE_ASM_FLAGS "-mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Wa,--warn -x assembler-with-cpp")
set(CMAKE_ASM_FLAGS_DEBUG "-O0 -ggdb")
set(CMAKE_ASM_FLAGS_RELEASE "-Os")

set(CMAKE_C_FLAGS "-mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-sp-d16 -ffunction-sections -fdata-sections -fstack-usage -Wall")
set(CMAKE_C_FLAGS_DEBUG "-O0 -ggdb")
set(CMAKE_C_FLAGS_RELEASE "-Os")

set(CMAKE_CXX_FLAGS "-mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-sp-d16 -ffunction-sections -fdata-sections -fstack-usage -fno-rtti -fno-exceptions -fno-threadsafe-statics -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-Os")

set(CMAKE_EXE_LINKER_FLAGS "-mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-sp-d16 -T\"../stm32h7_flash.ld\" -specs=nosys.specs -static -Wl,-cref,-u,Reset_Handler \"-Wl,-Map=canusbfdiso.map\" -Wl,--gc-sections -Wl,--defsym=malloc_getpagesize_P=0x1000 -Wl,--start-group -lc -lm -lstdc++ -lsupc++ -Wl,--end-group")


include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_subdirectory(stm32cube_fw_h7_v1_3_0)

add_executable(canusbfdiso
	src/main.cpp
	#src/startup_stm32h750xx.s
	src/syscalls.c
	#src/system_stm32h7xx.c
	src/tiny_printf.c
)

target_link_libraries(canusbfdiso
	c
	m
	stdc++
	supc++

	stm32h7xx_hal
	stm32h7xx_ll
	stm32h750xx_cmsis
)