if [ -d build ]; then rm -rf build; fi

BASE_PATH=$PWD

mkdir -p build/ram/release
pushd build/ram/release
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=$BASE_PATH/Toolchain_stm32h750_atollic_ram.cmake $BASE_PATH
popd

mkdir -p build/ram/debug
pushd build/ram/debug
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=$BASE_PATH/Toolchain_stm32h750_atollic_ram.cmake $BASE_PATH
popd

mkdir -p build/flash/release
pushd build/flash/release
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=$BASE_PATH/Toolchain_stm32h750_atollic_flash.cmake $BASE_PATH
popd

mkdir -p build/flash/debug
pushd build/flash/debug
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=$BASE_PATH/Toolchain_stm32h750_atollic_flash.cmake $BASE_PATH
popd