#!/bin/bash

# update packages
sudo apt-get update

# gcc
sudo apt install gcc-11 g++-11
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 100 --slave /usr/bin/g++ g++ /usr/bin/g++-11 --slave /usr/bin/gcov gcov /usr/bin/gcov-11
sudo update-alternatives --set gcc /usr/bin/gcc-11

# ninja
sudo apt-get install ninja-build

# python
sudo apt-get install libpython-dev
