#!/bin/bash

scp root@10.170.111.80:/home/root/fftbench/data/fftbench.csv data  # atom
python3 tools/show_benchmark_results.py