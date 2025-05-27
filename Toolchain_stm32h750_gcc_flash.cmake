set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR arm-none-eabi)

set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_C_COMPILER   arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)

set(CMAKE_CXX_STANDARD   14)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD     11)
set(CMAKE_C_EXTENSIONS   OFF)

set( ARCH_STM32 "-mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-sp-d16 -fstrict-volatile-bitfields")
set( FLAGS_STM32 "${ARCH_STM32} -ffunction-sections -fdata-sections" )

set( WARN_STM32 "-Wall -Werror=return-type" )

set(CMAKE_ASM_FLAGS 				"${FLAGS_STM32} ${WARN_STM32} -Wa,--warn -x assembler-with-cpp" CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS_DEBUG 			"-Og -g3" CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS_RELEASE 		"-O1" CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS_RELWITHDEBINFO 	"-O1 -g3" CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS_MINSIZEREL 		"-O1" CACHE STRING "" FORCE)

set(CMAKE_C_FLAGS 					"${FLAGS_STM32} ${WARN_STM32} -fstack-usage --specs=nano.specs" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_DEBUG 			"-Og -g3" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_RELEASE 			"-Os" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_RELWITHDEBINFO 	"-Os -g3" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_MINSIZEREL 		"-Os" CACHE STRING "" FORCE)

set(CMAKE_CXX_FLAGS 				"${FLAGS_STM32} ${WARN_STM32} -fstack-usage -fno-threadsafe-statics --specs=nano.specs" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG 			"-Og -g3" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE 		"-Os" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO 	"-Os -g3" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_MINSIZEREL 		"-Os" CACHE STRING "" FORCE)

set(CMAKE_EXE_LINKER_FLAGS "${ARCH_STM32} -T\"${CMAKE_CURRENT_SOURCE_DIR}/STM32H750VB_FLASH.ld\" -u _printf_float --specs=nosys.specs -static -Wl,-cref,-u,Reset_Handler \"-Wl,-Map=canusbfdiso.map\" -Wl,--gc-sections -Wl,--defsym=malloc_getpagesize_P=0x80" CACHE STRING "" FORCE)
