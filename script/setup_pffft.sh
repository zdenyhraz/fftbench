#!/bin/bash

clone --recursive https://github.com/marton78/pffft.git   
mkdir build && cd build

cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release -j 6
sudo ninja install
