cmake_minimum_required(VERSION 2.8)

project(fileclient)

set(CMAKE_CXX_STANDARD 11)

set(SOURCES
    main.cpp
)

set(CMAKE_BUILD_TYPE Debug)
add_compile_options(-g)
add_compile_options(-std=gnu++11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -g -Wall -O0 -Wno-unused-variable -pthread")

add_executable(fileclient ${SOURCES})
