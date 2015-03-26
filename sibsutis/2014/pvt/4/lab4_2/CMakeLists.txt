cmake_minimum_required(VERSION 2.8.4)
project(lab4)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

set(SOURCE_FILES main.cpp lock_free_queue.hpp lock_free_mutex.hpp)
add_executable(lab4 ${SOURCE_FILES})