#include <iostream>
#include <vector>
#include <string>
#include <exception>

#include <fmt/format.h>
#include <benchmark/benchmark.h>
#include <opencv2/opencv.hpp>
#include <fftw3.h>
#include <pocketfft/pocketfft_hdronly.h>
#include <pffft.hpp>

using i8 = int8_t;
using u8 = uint8_t;
using i16 = int16_t;
using u16 = uint16_t;
using i32 = int32_t;
using u32 = uint32_t;
using i64 = int64_t;
using u64 = uint64_t;
using f32 = float;
using f64 = double;
using f128 = long double;
using usize = size_t;

#include "Math.hpp"