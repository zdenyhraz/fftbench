cmake_minimum_required(VERSION 3.7)
project(fftbench)

# variables
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(ENABLE_PROFILING OFF)
set(ENABLE_SANITIZER OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(ENABLE_PCH ON)
set(ENABLE_OPENCV OFF)

# targets
add_executable(fftbench)

# compiler flags
target_compile_options(fftbench PRIVATE
-march=atom -fopenmp 
-Wall -Werror -Wfatal-errors -Wextra -Wpedantic 
#-Wshadow 
-Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function 
$<$<CONFIG:DEBUG>:-g;-O0>
$<$<CONFIG:RELEASE>:-O3>)

# ignore warnings from external sources
file(GLOB_RECURSE SRC_LIBS CONFIGURE_DEPENDS libs/*.h libs/*.hpp libs/*.c libs/*.cpp)
set_source_files_properties(${SRC_LIBS} PROPERTIES COMPILE_FLAGS -w)

# ccache
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
  set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
endif()

# fmt
add_subdirectory(libs/fmt)
include_directories(libs/fmt/include)
target_link_libraries(fftbench PRIVATE fmt::fmt-header-only)

# benchmark
set(BENCHMARK_ENABLE_TESTING OFF)
add_subdirectory(libs/benchmark)
target_link_libraries(fftbench PRIVATE benchmark::benchmark)

# opencv
find_package(OpenCV)
if (ENABLE_OPENCV AND OpenCV_FOUND)
  add_compile_definitions(ENABLE_OPENCV)
  include_directories(${OpenCV_INCLUDE_DIRS})
  target_link_libraries(fftbench PRIVATE ${OpenCV_LIBS})
endif()

# fftw
set(ENABLE_FLOAT ON CACHE BOOL "FFTW: Single-precision")
set(ENABLE_SSE ON CACHE BOOL "FFTW: Compile with SSE instruction set support")
set(ENABLE_SSE2 ON CACHE BOOL "FFTW: Compile with SSE2 instruction set support")
set(ENABLE_AVX ON CACHE BOOL "FFTW: Compile with AVX instruction set support")
set(ENABLE_AVX2 ON CACHE BOOL "FFTW: Compile with AVX2 instruction set support")
add_subdirectory(libs/fftw)
target_link_libraries(fftbench PRIVATE fftw3f)

# pffft
set(USE_SIMD ON CACHE BOOL "PFFFT: Use SIMD (SSE/AVX/NEON/ALTIVEC) CPU features")
add_subdirectory(libs/pffft)
target_compile_options(PFFFT PRIVATE -w)
target_link_libraries(fftbench PRIVATE PFFFT)

# pocketfft
add_compile_definitions(POCKETFFT_NO_MULTITHREADING)

# kfr
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  add_compile_definitions(ENABLE_KFR)
  add_subdirectory(libs/kfr)
  target_compile_options(kfr PRIVATE -w)
  target_compile_options(kfr_dft PRIVATE -w)
  target_compile_options(kfr_io PRIVATE -w)
  target_link_libraries(fftbench PRIVATE kfr)
  target_link_libraries(fftbench PRIVATE kfr_dft)
  target_link_libraries(fftbench PRIVATE kfr_io)
endif()

# custom
include_directories(src)
include_directories(libs)

# sources
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS src/*.hpp src/*.cpp)
target_sources(fftbench PRIVATE ${SRC})

# status messages
message(STATUS "CMAKE_CXX_COMPILER: " ${CMAKE_CXX_COMPILER_ID} " " ${CMAKE_CXX_COMPILER_VERSION})
message(STATUS "CMAKE_C_COMPILER: " ${CMAKE_C_COMPILER_ID} " " ${CMAKE_C_COMPILER_VERSION})
message(STATUS "Build type: " ${CMAKE_BUILD_TYPE})
message(STATUS "Compiler flags:" ${CMAKE_CXX_COMPILE_FLAGS})
message(STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS})
message(STATUS "CMAKE_CXX_FLAGS_DEBUG: " ${CMAKE_CXX_FLAGS_DEBUG})
message(STATUS "CMAKE_CXX_FLAGS_RELEASE: " ${CMAKE_CXX_FLAGS_RELEASE})
