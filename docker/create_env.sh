#!/bin/bash

# install ssh
apt update && apt install openssh-server sudo -y

useradd -m utbot
usermod -aG sudo utbot
passwd utbot

service ssh start

export OPERATING_SYSTEM_TAG=$(eval lsb_release -sr)
export LLVM_VERSION_MAJOR=14

export UTBOT_ALL=/utbot_distr
export UTBOT_INSTALL_DIR=$UTBOT_ALL/install
export UTBOT_CMAKE_BINARY=$UTBOT_INSTALL_DIR/bin/cmake

su - utbot
chsh -s /bin/bash

#RUN echo "Set disable_coredump false" >> /etc/sudo.conf

sudo apt update
sudo apt install vim
sudo apt install sudo file python3-dateutil wget fakeroot libssl-dev build-essential software-properties-common libtcmalloc-minimal4

#echo "check_certificate = off" > /etc/wgetrc

if [[ "$OPERATING_SYSTEM_TAG" = "18.04" ]]; then
  add-apt-repository ppa:ubuntu-toolchain-r/test
fi

sudo apt install gcc-9 g++-9 gcc-multilib g++-multilib gcc-9-multilib g++-9-multilib
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 100
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 100
sudo update-alternatives --install /usr/bin/gcov gcov /usr/bin/gcov-9 100

sudo apt install git libcurl4-openssl-dev

sudo apt install ninja-build python3-setuptools curl libcap-dev libncurses5-dev unzip libtcmalloc-minimal4 libgoogle-perftools-dev libsqlite3-dev doxygen python3-pip autoconf libtool

#sudo apt install libxshmfence1 libglu1 libgconf-2-4 libatk1.0-0 libatk-bridge2.0-0 libgdk-pixbuf2.0-0 libgtk-3-0 libgbm-dev libnss3-dev libxss-dev libasound2 xvfb

sudo mkdir $UTBOT_ALL
sudo chmod 777 $UTBOT_ALL -R
sudo chown utbot $UTBOT_ALL -R
cd $UTBOT_ALL

#Install cmake
wget https://github.com/Kitware/CMake/releases/download/v3.17.2/cmake-3.17.2.tar.gz -O /tmp/cmake_src.tar.gz
tar xfz /tmp/cmake_src.tar.gz -C $UTBOT_ALL
rm /tmp/cmake_src.tar.gz
cd $UTBOT_ALL/cmake-3.17.2
./bootstrap --prefix=$UTBOT_INSTALL_DIR --parallel=$(nproc)
make -j$(nproc)
make install
cd $UTBOT_ALL
rm -rf $UTBOT_ALL/cmake-3.17.2

sudo pip3 install tabulate==0.8.7 \
  typing==3.7.4.3 \
  lit==17.0.6 \
  wllvm==1.3.1

mkdir $UTBOT_ALL/llvm_gold_plugin
wget -P $UTBOT_ALL/llvm_gold_plugin https://raw.githubusercontent.com/bminor/binutils-gdb/fd67aa1129fd006ad49ed5ecb2b063705211553a/include/plugin-api.h
git clone --single-branch --branch "release/${LLVM_VERSION_MAJOR}.x" --depth 1 "https://github.com/llvm/llvm-project.git" $UTBOT_ALL/llvm-project
mkdir -p $UTBOT_ALL/llvm-project/build
cd $UTBOT_ALL/llvm-project/build
$UTBOT_CMAKE_BINARY \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=$UTBOT_INSTALL_DIR \
  -DLLVM_INCLUDE_TESTS=OFF \
  -DLLVM_BINUTILS_INCDIR=$UTBOT_ALL/llvm_gold_plugin \
  -DLLVM_ENABLE_RTTI=ON \
  -DLLVM_ENABLE_EH=ON \
  -DLLVM_TARGETS_TO_BUILD="host" \
  -DLLVM_INSTALL_UTILS=ON \
  -DLLVM_ENABLE_PROJECTS="clang;libclc;lld;lldb;clang-tools-extra" \
  -DLLVM_ENABLE_RUNTIMES="compiler-rt;libc;libcxx;libcxxabi" \
  -G "Ninja" ../llvm
$UTBOT_CMAKE_BINARY --build . --target install

#export CFLAGS="-gdwarf-4"
#export CXXFLAGS="-gdwarf-4"

export LLVM_COMPILER_PATH=$UTBOT_INSTALL_DIR/bin
export LLVM_COMPILER=clang

mkdir -p $UTBOT_ALL/llvm-project/libcxx_build
cd $UTBOT_ALL/llvm-project/libcxx_build
CC=wllvm CXX=wllvm++ $UTBOT_CMAKE_BINARY \
  -DLLVM_ENABLE_PROJECTS="libcxx;libcxxabi" \
  -DLLVM_ENABLE_THREADS:BOOL=OFF \
  -DLIBCXX_ENABLE_THREADS:BOOL=OFF \
  -DLIBCXX_ENABLE_SHARED:BOOL=ON \
  -DLIBCXXABI_ENABLE_THREADS:BOOL=OFF \
  -DCMAKE_BUILD_TYPE:STRING=RelWithDebInfo \
  -DLLVM_TARGETS_TO_BUILD=host \
  -DCMAKE_INSTALL_PREFIX=$UTBOT_ALL/libcxx/install \
  -DLIBCXX_ENABLE_STATIC_ABI_LIBRARY:BOOL=ON ../llvm

CC=wllvm CXX=wllvm++ make cxx -j$(nproc)
cd projects
CC=wllvm CXX=wllvm++ make install
find $UTBOT_ALL/libcxx/install/lib/lib*.so -print0 | xargs -0 --max-args=1 extract-bc
find $UTBOT_ALL/libcxx/install/lib/lib*.so -print0 | xargs -0 --max-args=1 rm
find $UTBOT_ALL/libcxx/install/lib/lib*.a -print0 | xargs -0 --max-args=1 extract-bc
find $UTBOT_ALL/libcxx/install/lib/lib*.a -print0 | xargs -0 --max-args=1 rm
cp -R $UTBOT_ALL/llvm-project/libcxxabi $UTBOT_ALL/libcxx

cd $UTBOT_ALL
rm -rf $UTBOT_ALL/llvm-project
rm -rf $UTBOT_ALL/llvm_gold_plugin

#export CC=$UTBOT_INSTALL_DIR/bin/clang
#export CXX=$UTBOT_INSTALL_DIR/bin/clang++

#Install GRPC
git clone --single-branch -b v1.49.0 --depth=1 https://github.com/grpc/grpc --recurse-submodules -j4 $UTBOT_ALL/grpc
mkdir -p $UTBOT_ALL/grpc/cmake_build
cd $UTBOT_ALL/grpc/cmake_build
$UTBOT_CMAKE_BINARY \
  -DgRPC_INSTALL=ON \
  -DgRPC_BUILD_TESTS=OFF \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=$UTBOT_INSTALL_DIR \
  -G "Ninja" ..
#make -j$(nproc)
#make install
$UTBOT_CMAKE_BINARY --build . --target install
cd $UTBOT_ALL
rm -rf $UTBOT_ALL/grpc

#Instal gtest
git clone --single-branch -b release-1.10.0 https://github.com/google/googletest.git $UTBOT_ALL/gtest
mkdir -p $UTBOT_ALL/gtest/build
cd $UTBOT_ALL/gtest/build
$UTBOT_CMAKE_BINARY -G "Ninja" -DCMAKE_INSTALL_PREFIX=$UTBOT_INSTALL_DIR ..
$UTBOT_CMAKE_BINARY --build . --target install
cd $UTBOT_ALL

#Install z3
git clone --single-branch -b z3-4.8.17 --depth=1 https://github.com/Z3Prover/z3.git $UTBOT_ALL/z3-src
mkdir -p $UTBOT_ALL/z3-src/build
cd $UTBOT_ALL/z3-src/build
$UTBOT_CMAKE_BINARY -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$UTBOT_INSTALL_DIR ..
$UTBOT_CMAKE_BINARY --build . --target install
cd $UTBOT_ALL
rm -rf $UTBOT_ALL/z3-src

#install bitwuzla
#export PATH=$PATH:/utbot_distr/install/bin
#sudo pip3 install meson
#sudo apt install pkg-config libgmp-dev
#git clone --single-branch -b 0.3.1 --depth=1 https://github.com/bitwuzla/bitwuzla.git $UTBOT_ALL/bitwuzla
#cd $UTBOT_ALL/bitwuzla
#./configure.py --build-dir $UTBOT_ALL/bitwuzla/build --prefix $UTBOT_INSTALL_DIR --static --no-unit-testing
#cd $UTBOT_ALL/bitwuzla/build
#ninja .
#ninja install
#cd $UTBOT_ALL
#rm -rf $UTBOT_ALL/bitwuzla

if [[ "$OPERATING_SYSTEM_TAG" = "18.04" ]]; then
  sudo apt install nodejs-dev node-gyp libssl1.0-dev
fi
sudo apt install nodejs npm openssh-server net-tools gdb vim-nox rsync
sudo pip3 install git+https://chromium.googlesource.com/external/gyp

# Update node and npm
npm cache clean -f
sudo -E npm install -g n
sudo -E n 16
sudo -E apt remove -y --purge nodejs npm

# Install cmake which can generate link_commands.json. Installing cmake the second time in order to build base image faster since this cmake may be changed frequently.
git clone --single-branch -b utbot-0.1.2b --depth=1 https://github.com/Software-Analysis-Team/CMake.git $UTBOT_ALL/cmake
cd $UTBOT_ALL/cmake
$UTBOT_CMAKE_BINARY -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$UTBOT_INSTALL_DIR .
$UTBOT_CMAKE_BINARY --build . --target install
cd $UTBOT_ALL
rm -rf $UTBOT_ALL/cmake

mkdir $UTBOT_ALL/klee
mkdir $UTBOT_ALL/server-install

sudo pip3 install lit

wget https://github.com/CLIUtils/CLI11/releases/download/v1.9.1/CLI11.hpp -P $UTBOT_ALL/cli
wget https://github.com/agauniyal/rang/releases/download/v3.1.0/rang.hpp -P $UTBOT_ALL/cli

#COPY building_dependencies/runtime_env.sh /home/utbot/.bashrc
#COPY building_dependencies/runtime_env.sh /root/.bashrc

sudo update-alternatives --install /usr/bin/python python /usr/bin/python3 100
git clone -b klee_uclibc_v1.2 https://github.com/klee/klee-uclibc.git $UTBOT_ALL/klee-uclibc-32
cp $UTBOT_ALL/klee-uclibc-32 $UTBOT_ALL/klee-uclibc-64 -R
cd $UTBOT_ALL/klee-uclibc-32
./configure --make-llvm-lib
make KLEE_CFLAGS="-m32" -j$(nproc)

cd $UTBOT_ALL/klee-uclibc-64
./configure --make-llvm-lib
make -j$(nproc)

# Download library for access private members
git clone https://github.com/martong/access_private.git $UTBOT_ALL/access_private


sudo update-alternatives --install /usr/bin/cc cc $UTBOT_INSTALL_DIR/bin/clang 100
sudo update-alternatives --install /usr/bin/c++ c++ $UTBOT_INSTALL_DIR/bin/clang++ 100
