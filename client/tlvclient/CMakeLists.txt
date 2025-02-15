cmake_minimum_required(VERSION 2.8)

project(netserver)

set(CMAKE_CXX_STANDARD 11)

include_directories(${PROJECT_SOURCE_DIR}/net)

set(SOURCES
    net/Communicator.cpp
    main.cpp
)

set(CMAKE_BUILD_TYPE Debug)
add_compile_options(-g)
add_compile_options(-std=gnu++11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -g -Wall -O0 -Wno-unused-variable -pthread")


add_executable(client ${SOURCES})

