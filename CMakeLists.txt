cmake_minimum_required(VERSION 3.11)
project(fftbench)

# variables
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# targets
add_executable(fft_bench)
add_executable(fftw_memory)
add_executable(fftw_wisdom)
add_executable(ipp_memory)

# compiler flags
target_compile_options(fft_bench PRIVATE -march=native -fopenmp -pthread)
target_compile_options(fft_bench PRIVATE "$<$<CONFIG:DEBUG>:-g;-O0>")
target_compile_options(fft_bench PRIVATE "$<$<CONFIG:RELEASE>:-O3>")
target_compile_options(fft_bench PRIVATE -Wall -Werror -Wfatal-errors -Wextra -Wpedantic) #-Wshadow
target_compile_options(fft_bench PRIVATE -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function)

# ccache
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
  set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
endif()

# OpenMP
find_package(OpenMP)
if (OpenMP_CXX_FOUND)
  target_link_libraries(fft_bench OpenMP::OpenMP_CXX)
  target_link_libraries(fftw_memory OpenMP::OpenMP_CXX)
  target_link_libraries(fftw_wisdom OpenMP::OpenMP_CXX)
  target_link_libraries(ipp_memory OpenMP::OpenMP_CXX)
endif()

# fmt
add_subdirectory(libs/fmt)
include_directories(libs/fmt/include)
target_link_libraries(fft_bench fmt::fmt)
target_link_libraries(fftw_wisdom fmt::fmt)

# benchmark
set(BENCHMARK_ENABLE_TESTING OFF)
add_subdirectory(libs/benchmark)
target_link_libraries(fft_bench benchmark::benchmark)

# opencv
find_package(OpenCV)
if (OpenCV_FOUND)
  add_compile_definitions(ENABLE_OPENCV)
  include_directories(${OpenCV_INCLUDE_DIRS})
  target_link_libraries(fft_bench ${OpenCV_LIBS})
endif()

# fftw
set(ENABLE_OPENMP ON CACHE BOOL "FFTW: Use OpenMP for multithreading")
set(ENABLE_THREADS ON CACHE BOOL "FFTW: Use pthread for multithreading")
set(ENABLE_FLOAT ON CACHE BOOL "FFTW: Single-precision")
set(ENABLE_SSE ON CACHE BOOL "FFTW: Compile with SSE instruction set support")
set(ENABLE_SSE2 ON CACHE BOOL "FFTW: Compile with SSE2 instruction set support")
set(ENABLE_AVX OFF CACHE BOOL "FFTW: Compile with AVX instruction set support")
set(ENABLE_AVX2 OFF CACHE BOOL "FFTW: Compile with AVX2 instruction set support")
add_subdirectory(libs/fftw)
if (ENABLE_OPENMP AND OpenMP_CXX_FOUND)
  message(STATUS "FFTW: Using OpenMP threads")
  target_link_libraries(fft_bench fftw3f_omp)
  target_link_libraries(fftw_memory fftw3f_omp)
  target_link_libraries(fftw_wisdom fftw3f_omp)
else()
  message(STATUS "FFTW: Using pthread threads")
  target_link_libraries(fft_bench fftw3f_threads)
  target_link_libraries(fftw_memory fftw3f_threads)
  target_link_libraries(fftw_wisdom fftw3f_threads)
endif()
target_link_libraries(fft_bench fftw3f m)
target_link_libraries(fftw_memory fftw3f m) 
target_link_libraries(fftw_wisdom fftw3f m)

# pffft
set(USE_SIMD ON CACHE BOOL "PFFFT: Use SIMD (SSE/AVX/NEON/ALTIVEC) CPU features")
add_subdirectory(libs/pffft)
target_compile_options(PFFFT PRIVATE -w)
target_link_libraries(fft_bench PFFFT)

# pocketfft
add_compile_definitions(POCKETFFT_NO_MULTITHREADING)

# kfr
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_compile_definitions(ENABLE_KFR)
  add_subdirectory(libs/kfr)
  target_compile_options(kfr INTERFACE -w)
  target_compile_options(kfr_dft PRIVATE -w)
  target_compile_options(kfr_io PRIVATE -w)
  target_link_libraries(fft_bench kfr)
  target_link_libraries(fft_bench kfr_dft)
  target_link_libraries(fft_bench kfr_io)
endif()

# ipp
find_package(IPP REQUIRED PATHS libs/intel/oneapi/ipp/latest/lib/cmake/ipp)
if (IPP_FOUND)
  add_compile_definitions(ENABLE_IPP)
  include_directories(libs/intel/oneapi/ipp/latest/include)
  target_link_libraries(fft_bench IPP::ipps)
  target_link_libraries(ipp_memory IPP::ipps)
  message(STATUS "IPP_ARCH: " ${IPP_ARCH})
  message(STATUS "IPP_TL_VARIANT: " ${IPP_TL_VARIANT})
endif()

# custom
include_directories(src)
include_directories(libs)

# sources
target_sources(fft_bench PRIVATE src/fft_bench/Main.cpp)
target_sources(fftw_memory PRIVATE src/fftw_memory/Main.cpp)
target_sources(fftw_wisdom PRIVATE src/fftw_wisdom/Main.cpp)
target_sources(ipp_memory PRIVATE src/ipp_memory/Main.cpp)

# status messages
message(STATUS "CMAKE_CXX_COMPILER: " ${CMAKE_CXX_COMPILER_ID} " " ${CMAKE_CXX_COMPILER_VERSION})
message(STATUS "CMAKE_C_COMPILER: " ${CMAKE_C_COMPILER_ID} " " ${CMAKE_C_COMPILER_VERSION})
message(STATUS "CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})
message(STATUS "CMAKE_CXX_COMPILE_FLAGS:" ${CMAKE_CXX_COMPILE_FLAGS})
message(STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS})
message(STATUS "CMAKE_CXX_FLAGS_DEBUG: " ${CMAKE_CXX_FLAGS_DEBUG})
message(STATUS "CMAKE_CXX_FLAGS_RELEASE: " ${CMAKE_CXX_FLAGS_RELEASE})
