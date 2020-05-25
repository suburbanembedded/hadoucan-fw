cmake_minimum_required(VERSION 3.5)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/cmake-modules/")
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

project(hadoucan-fw C CXX ASM)

find_package(Doxygen
             REQUIRED dot
             OPTIONAL_COMPONENTS mscgen)

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
	hadoucan-fw/include
	external/
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

add_subdirectory(hadoucan-fw)
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

