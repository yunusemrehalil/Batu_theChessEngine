cmake_minimum_required(VERSION 3.16)
project(BatuChessEngine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Optimization flags for Release build
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

# Main executable
add_executable(batu main.cpp)

# Include directories
target_include_directories(batu PRIVATE 
    ${CMAKE_SOURCE_DIR}/headers
    ${CMAKE_SOURCE_DIR}/include
)

# Compiler warnings
if(MSVC)
    target_compile_options(batu PRIVATE /W4)
else()
    target_compile_options(batu PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Build type default to Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
