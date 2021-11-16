#
# Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
#

# This script downloads debian packages.
# It installs libc6-dev package (which is dev version of libc) into $UTBOT_ALL/debian-libc-dev-install and other packages into $UTBOT_ALL/debs-install

set -e
set -o pipefail

# Downloading apt-rdepends tool which can get all the dependencies for a package
apt-get update && apt-get  -y --no-install-recommends install apt-rdepends && apt-get update
rm -rf /tmp/debian_dependencies && mkdir -p /tmp/debian_dependencies && cd /tmp/debian_dependencies
# expand_aliases is used to support alias command properly in bash script
shopt -s expand_aliases
# A grep command which clears out the output of apt-rdepends
alias grepdepends='grep -v "^ " | grep -v "^libc-dev$" | grep -v "^debconf-2.0$" | grep -v "^libc6$" | grep -v "^libunwind8-dev$" | grep -v "^awk$"'
# Get all the dependencies of utbot
apt-rdepends libsqlite3-dev libgoogle-perftools-dev libssl-dev python3-pip gzip make gcc-9 g++-9 | grepdepends > all.txt
# Get all the dependencies of libc6-dev
apt-rdepends libc6-dev | grepdepends > debian-libc-dev.txt
# Get all the dependencies of utbot except all the dependencies of libc6-dev
diff --new-line-format="" --unchanged-line-format="" <(sort all.txt) <(sort debian-libc-dev.txt) > all_without_libc-dev.txt || :

# A function which downloads all the dependencies from text file and extracts them.
# Prerequisites: file path/to/file.txt exists
# Arguments:
#   $1 = path/to/file  The first argument is a path to a file without the .txt extension
# The extracted packages will be located in $UTBOT_ALL/path/to/file-install directory
get_debian_packages() {
  # Create a directory for .deb packages
  rm -rf $1 && mkdir -p $1 && cd $1
  # Download .deb packages
  apt-get download $(cat ../$1.txt)
  cd ..

  # Extract all the .deb packages into $UTBOT_ALL/$1-install directory
  for filename in $1/*.deb; do
          dpkg-deb -x "$filename" $UTBOT_ALL/$1-install
  done
}

# Get all packages except libc6-dev
get_debian_packages all_without_libc-dev
rm -rf $UTBOT_ALL/debs-install
mv $UTBOT_ALL/all_without_libc-dev-install $UTBOT_ALL/debs-install
# Get libc6-dev package and it's dependencies
get_debian_packages debian-libc-dev

# Creating links to the current versions of gcc and python
cd $UTBOT_ALL/debs-install/usr/bin
ln -s python3 python
ln -s gcov-9 gcov
ln -s g++-9 g++
ln -s gcc-9 gcc

# Setup python packages
pip3 install --target=$UTBOT_ALL/debs-install/usr/local/lib/python3.4/dist-packages/ --ignore-installed typing
