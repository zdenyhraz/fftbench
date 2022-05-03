#!/bin/bash

wget https://www.fftw.org/fftw-3.3.10.tar.gz
sudo chmod 755 fftw-3.3.10.tar.gz
tar -xf fftw-3.3.10.tar.gz && mv fftw-3.3.10 fftw && cd fftw
mkdir build && cd build

# double
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release -DENABLE_AVX=ON -DENABLE_AVX2=ON -DENABLE_SSE=ON -DENABLE_SSE2=ON 
cmake --build . --config Release -j 6
sudo ninja install

# float
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release -DENABLE_FLOAT=ON -DENABLE_AVX=ON -DENABLE_AVX2=ON -DENABLE_SSE=ON -DENABLE_SSE2=ON 
cmake --build . --config Release -j 6
sudo ninja install
