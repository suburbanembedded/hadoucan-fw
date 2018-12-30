mkdir -p build/release
pushd build/release
cmake -DCMAKE_BUILD_TYPE=Release ../..
popd

mkdir -p build/debug
pushd build/debug
cmake -DCMAKE_BUILD_TYPE=Debug ../..
popd