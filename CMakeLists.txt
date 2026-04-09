cmake_minimum_required(VERSION 3.15)
project(pinnacleSets)
set(CMAKE_CXX_STANDARD 20)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if(MSVC)
    # Use MSVC-specific flags (/W4 is roughly equivalent to -Wextra)
    set(pinnacleSets PRIVATE /W4)
    set(CMAKE_CXX_FLAGS_RELEASE "-O2")
else()
    # Use GCC/Clang flags
    set(CMAKE_CXX_FLAGS "-Wall -Wextra")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

set(SOURCE_FILES
    src/main.cpp
    src/permutations.cpp
    src/CompleteGraph.cpp
    src/graph.hpp
)

add_executable(pinnacleSets ${SOURCE_FILES})
