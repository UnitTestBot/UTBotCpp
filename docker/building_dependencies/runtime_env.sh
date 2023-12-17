# This script is used to set environment variables inside developer environment

# Common env
export UTBOT_ALL=/utbot_distr
export UTBOT_INSTALL_DIR=$UTBOT_ALL/install
export UTBOT_CMAKE_BINARY=$UTBOT_INSTALL_DIR/bin/cmake
export C_INCLUDE_PATH=$UTBOT_INSTALL_DIR/lib/clang/14.0.6/include/:$UTBOT_ALL/gtest/googletest/include
export LLVM_INCLUDE_DIRS=$UTBOT_ALL/llvm/llvm
export CMAKE_CXX_STANDARD=17
export CMAKE_CXX_STANDARD_REQUIRED=ON
export GRPC_PATH=$UTBOT_INSTALL_DIR
export CLI_PATH=$UTBOT_ALL/cli
export NODE_TLS_REJECT_UNAUTHORIZED=0

# KLEE env
export COVERAGE=0
export USE_TCMALLOC=1
export LLVM_VERSION=14.0
export ENABLE_OPTIMIZED=1
export ENABLE_DEBUG=1
export DISABLE_ASSERTIONS=0
export REQUIRES_RTTI=0
export SOLVERS=Z3
export GTEST_VERSION=1.10.0
export TCMALLOC_VERSION=2.7
export SANITIZER_BUILD=
export MINISAT_VERSION=master
export USE_LIBCXX=1
export KLEE_RUNTIME_BUILD="Debug+Asserts"

export CC=$UTBOT_INSTALL_DIR/bin/clang
export CXX=$UTBOT_INSTALL_DIR/bin/clang++
export CPATH=$CPATH:$UTBOT_ALL/klee/include
export LD_LIBRARY_PATH=$UTBOT_INSTALL_DIR/lib
export LDFLAGS='-fuse-ld=gold'
export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:$UTBOT_ALL/bear/bin:$UTBOT_ALL/klee/bin:$UTBOT_INSTALL_DIR/bin:$PATH

if [ -z "${VERSION}" ]
then
    export VERSION=$(date '+%Y.%-m').0
fi
