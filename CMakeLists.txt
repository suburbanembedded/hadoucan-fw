project(CANUSBFDISO C CXX ASM)

set(CXX_STANDARD 11)
set(C_STANDARD 99)

add_definition(STM32H750xx)

set(CMAKE_ASM_FLAGS "-mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Wa,--warn -x assembler-with-cpp")
set(CMAKE_ASM_FLAGS_DEBUG "-O0 -ggdb")
set(CMAKE_ASM_FLAGS_RELEASE "-Os")

set(CMAKE_C_FLAGS "-mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-sp-d16 -ffunction-sections -fdata-sections -fstack-usage -Wall")
set(CMAKE_C_FLAGS_DEBUG "-O0 -ggdb")
set(CMAKE_C_FLAGS_RELEASE "-Os")

set(CMAKE_CXX_FLAGS "-mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-sp-d16 -ffunction-sections -fdata-sections -fstack-usage -fno-rtti -fno-exceptions -fno-threadsafe-statics -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-Os")

add_executable(canusbfdiso

)