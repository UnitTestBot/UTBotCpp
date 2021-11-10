mkdir -p build
cd build
$UTBOT_CMAKE_BINARY -G "Ninja" -DCMAKE_INSTALL_PREFIX=$UTBOT_ALL/server-install ..
$UTBOT_CMAKE_BINARY --build . --parallel `nproc`
$UTBOT_CMAKE_BINARY --install .

./UTBot_UnitTests info