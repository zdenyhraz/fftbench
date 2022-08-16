cmake_minimum_required(VERSION 3.11)
project(fftbench)

# variables
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# targets
add_executable(fftbench)

# compiler flags
target_compile_options(fftbench PRIVATE -march=native -fopenmp)
target_compile_options(fftbench PRIVATE -Wall -Werror -Wfatal-errors -Wextra -Wpedantic) #-Wshadow
target_compile_options(fftbench PRIVATE -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function)
target_compile_options(fftbench PRIVATE "$<$<CONFIG:DEBUG>:-g;-O0>")
target_compile_options(fftbench PRIVATE "$<$<CONFIG:RELEASE>:-O3>")

# ccache
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
  set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
endif()

# fmt
add_subdirectory(libs/fmt)
include_directories(libs/fmt/include)
target_link_libraries(fftbench fmt::fmt-header-only)

# benchmark
set(BENCHMARK_ENABLE_TESTING OFF)
add_subdirectory(libs/benchmark)
target_link_libraries(fftbench benchmark::benchmark)

# opencv
find_package(OpenCV)
if (OpenCV_FOUND)
  add_compile_definitions(ENABLE_OPENCV)
  include_directories(${OpenCV_INCLUDE_DIRS})
  target_link_libraries(fftbench ${OpenCV_LIBS})
endif()

# fftw
set(ENABLE_FLOAT ON CACHE BOOL "FFTW: Single-precision")
set(ENABLE_SSE ON CACHE BOOL "FFTW: Compile with SSE instruction set support")
set(ENABLE_SSE2 ON CACHE BOOL "FFTW: Compile with SSE2 instruction set support")
set(ENABLE_AVX OFF CACHE BOOL "FFTW: Compile with AVX instruction set support")
set(ENABLE_AVX2 OFF CACHE BOOL "FFTW: Compile with AVX2 instruction set support")
add_subdirectory(libs/fftw)
target_link_libraries(fftbench fftw3f)

# pffft
set(USE_SIMD ON CACHE BOOL "PFFFT: Use SIMD (SSE/AVX/NEON/ALTIVEC) CPU features")
add_subdirectory(libs/pffft)
target_compile_options(PFFFT PRIVATE -w)
target_link_libraries(fftbench PFFFT)

# pocketfft
add_compile_definitions(POCKETFFT_NO_MULTITHREADING)

# kfr
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_compile_definitions(ENABLE_KFR)
  add_subdirectory(libs/kfr)
  target_compile_options(kfr INTERFACE -w)
  target_compile_options(kfr_dft PRIVATE -w)
  target_compile_options(kfr_io PRIVATE -w)
  target_link_libraries(fftbench kfr)
  target_link_libraries(fftbench kfr_dft)
  target_link_libraries(fftbench kfr_io)
endif()

# ipp
find_package(IPP REQUIRED PATHS libs/intel/oneapi/ipp/latest/lib/cmake/ipp)
if (IPP_FOUND)
  add_compile_definitions(ENABLE_IPP)
  include_directories(libs/intel/oneapi/ipp/latest/include)
  target_link_libraries(fftbench IPP::ippcore IPP::ippvm IPP::ipps)
  message(STATUS "IPP_ARCH: " ${IPP_ARCH})
  message(STATUS "IPP_TL_VARIANT: " ${IPP_TL_VARIANT})
endif()

# custom
include_directories(src)
include_directories(libs)

# sources
target_sources(fftbench PRIVATE src/Main.cpp)

# status messages
message(STATUS "CMAKE_CXX_COMPILER: " ${CMAKE_CXX_COMPILER_ID} " " ${CMAKE_CXX_COMPILER_VERSION})
message(STATUS "CMAKE_C_COMPILER: " ${CMAKE_C_COMPILER_ID} " " ${CMAKE_C_COMPILER_VERSION})
message(STATUS "Build type: " ${CMAKE_BUILD_TYPE})
message(STATUS "Compiler flags:" ${CMAKE_CXX_COMPILE_FLAGS})
message(STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS})
message(STATUS "CMAKE_CXX_FLAGS_DEBUG: " ${CMAKE_CXX_FLAGS_DEBUG})
message(STATUS "CMAKE_CXX_FLAGS_RELEASE: " ${CMAKE_CXX_FLAGS_RELEASE})
