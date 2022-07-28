# This script clears out all the unnecessary files from the release build

# Need just clang-10, llvm-cov, llvm-profdata and cmake in $UTBOT_INSTALL_DIR/bin
cd $UTBOT_INSTALL_DIR/bin && find -type f \( -name "*" ! -name "llvm-cov" ! -name "llvm-profdata"  ! -name "llvm-nm" ! -name "clang" ! -name "clang++" ! -name "clang-10" ! -name cmake \) -delete

# Delete all broken links. Don't need to delete clang which is a link (not broken) to clang-10
find $UTBOT_INSTALL_DIR/bin/ -xtype l -delete

# Delete all regular files in $UTBOT_INSTALL_DIR/lib except gold plugin, but not recursively
cd $UTBOT_INSTALL_DIR/lib && find -maxdepth 1 -type f \( -name "*" ! -name "LLVMgold.so" ! -name "libz3.so*" \) -delete

#TODO some libs are needed:
# Only certain libs should remain in $UTBOT_INSTALL_DIR/lib/clang/10.0.0/lib/linux
#cd $UTBOT_INSTALL_DIR/lib/clang/10.0.0/lib/linux &&  find -maxdepth 1 -type f \( -name "*" ! -name "libclang_rt.ubsan_standalone-x86_64.a" ! -name "libclang_rt.ubsan_standalone_cxx-x86_64.a" ! -name "libclang_rt.profile-x86_64.a" \) -delete

rm -rf $UTBOT_ALL/klee/bin

# There should be just some klee libraries in klee/lib
cd $UTBOT_ALL/klee/lib && find -type f \( ! -name '*_Debug+Asserts.bca' ! -name 'klee-uclibc.bca' \) -delete

# Previous command removed only regular files, need to remove also symlincs and then empty directories
find $UTBOT_ALL/klee -type l -delete
find $UTBOT_ALL/klee -type d -empty -delete

rm -rf $UTBOT_ALL/tsl
rm -rf $UTBOT_ALL/parallel_hashmap
rm -rf $UTBOT_ALL/cli
rm -rf $UTBOT_ALL/json
# we use klee libc
rm -rf $UTBOT_ALL/uclibc/
rm -rf $UTBOT_ALL/node_modules

# there should be error, it's okay, directory remains with some files
# dpkg should remain because it is useful for apt update
mv $UTBOT_ALL/debs-install/usr/share/dpkg ~ && rm -rf $UTBOT_ALL/debs-install/usr/share &&  mv ~/dpkg $UTBOT_ALL/debs-install/usr/share

rm -rf $UTBOT_INSTALL_DIR/include
