cmake_minimum_required(VERSION 2.8)

project(netserver)

set(CMAKE_CXX_STANDARD 11)

include_directories(${PROJECT_SOURCE_DIR}/kernel)
include_directories(${PROJECT_SOURCE_DIR}/commom)
include_directories(${PROJECT_SOURCE_DIR}/app)
include_directories(${PROJECT_SOURCE_DIR}/app/http)
include_directories(${PROJECT_SOURCE_DIR}/app/tlvmsg)
include_directories(${PROJECT_SOURCE_DIR}/app/file)


file(GLOB_RECURSE SRC_ALL_CPP_BASE
    ${PROJECT_SOURCE_DIR}/kernel/*.cpp
    ${PROJECT_SOURCE_DIR}/commom/*.cpp
    ${PROJECT_SOURCE_DIR}/app/*.cpp
    ${PROJECT_SOURCE_DIR}/app/http/*.cpp
    ${PROJECT_SOURCE_DIR}/app/tlvmsg/*.cpp
    ${PROJECT_SOURCE_DIR}/app/file/*.cpp
)

set(SOURCES
    ${SRC_ALL_CPP_BASE}
    mainfileserver.cpp
)

add_compile_options(-g)
add_compile_options(-std=gnu++11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x  -Wall -O0 -Wno-unused-variable -pthread")

add_executable(netserver ${SOURCES})
