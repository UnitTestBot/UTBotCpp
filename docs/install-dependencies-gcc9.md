# Install dependencies and GCC9

In your WSL or Docker container, install the required dependencies and the compiler:

```bash
sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
sudo apt-get update -y
sudo apt-get dist-upgrade -y
sudo apt-get install build-essential software-properties-common gcc-9 g++-9 -y
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 60 --slave /usr/bin/g++ g++ /usr/bin/g++-9
sudo update-alternatives --config gcc
```