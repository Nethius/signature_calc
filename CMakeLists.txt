cmake_minimum_required(VERSION 3.16)
project(signature_calc)

set(CMAKE_CXX_STANDARD 14)

add_executable(signature_calc main.cpp safeQueue.cpp safeQueue.h safeMap.cpp safeMap.h)