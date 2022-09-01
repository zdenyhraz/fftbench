#!/bin/bash

rm -f massif.out.*
valgrind --tool=massif --threshold=0 --detailed-freq=1 ./build/fft_fftw
valgrind --tool=massif --threshold=0 --detailed-freq=1 ./build/fft_ipp
