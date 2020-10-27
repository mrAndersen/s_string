cmake_minimum_required(VERSION 3.18)
project(s_search)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-lsodium")

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "-O2 -lsodium")
    set(CMAKE_C_FLAGS "-O2 -lsodium")
endif ()


add_executable(
        s_search

        hash_table.h
        hash_table.cpp

        main.cpp
)

target_link_libraries(
        s_search
        pthread
)