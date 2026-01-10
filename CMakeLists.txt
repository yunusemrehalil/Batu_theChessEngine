cmake_minimum_required(VERSION 3.16)
project(BatuChessEngine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Optimization flags for Release build
if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ob2 /DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "/Zi /Od")
else()
    # GCC/Clang optimizations: -O3, native CPU, fast-math, LTO
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -ffast-math -flto -DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
endif()

# Main executable
add_executable(batu main.cpp)

# Include directories
target_include_directories(batu PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
)

# Compiler-specific options
if(MSVC)
    # MSVC: warnings + fast floating point + AVX2 if available
    target_compile_options(batu PRIVATE /W4 /fp:fast)
    # Enable AVX2 for modern CPUs (comment out for older CPUs)
    # target_compile_options(batu PRIVATE /arch:AVX2)
else()
    target_compile_options(batu PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Enable Link-Time Optimization for Release
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET batu PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# Build type default to Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
