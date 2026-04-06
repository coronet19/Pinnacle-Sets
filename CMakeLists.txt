cmake_minimum_required(VERSION 3.15)
project(pinnacleSets)
set(CMAKE_CXX_STANDARD 20)

set(SOURCE_FILES src/main.cpp src/permutations.cpp src/CompleteGraph.cpp)

add_executable(pinnacleSets ${SOURCE_FILES})
