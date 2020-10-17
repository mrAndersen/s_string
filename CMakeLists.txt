cmake_minimum_required(VERSION 3.18)
project(s_search)

set(CMAKE_CXX_STANDARD 14)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "-O2")
    set(CMAKE_C_FLAGS "-O2")
endif ()

add_executable(
        s_search
        main.cpp
)

target_link_libraries(
        s_search
        pthread
)