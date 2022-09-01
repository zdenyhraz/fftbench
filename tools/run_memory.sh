#!/bin/bash

valgrind --tool=massif ./build/fft_fftw
valgrind --tool=massif ./build/fft_ipp
