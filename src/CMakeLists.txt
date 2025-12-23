cmake_minimum_required(VERSION 3.30)
project(stock)

set(CMAKE_CXX_STANDARD 23)

add_executable(stock ../main.cpp Exchange.cpp)