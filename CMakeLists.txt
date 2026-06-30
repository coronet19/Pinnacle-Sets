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
    src/tui.hpp
)

add_executable(pinnacleSets ${SOURCE_FILES})

# Link the standard libraries and the runtime statically so the binary
# has no shared-library dependencies.
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(pinnacleSets PRIVATE Threads::Threads)

if(MSVC)
    # Use the static C runtime.
    set_property(TARGET pinnacleSets PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
    # -static pulls in static libc/libstdc++/libgcc; the whole-archive
    # wrapping forces pthread symbols to be retained for a static build.
    target_link_options(pinnacleSets PRIVATE
        -static -static-libgcc -static-libstdc++
        -Wl,--whole-archive -lpthread -Wl,--no-whole-archive)
endif()
