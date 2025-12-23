cmake_minimum_required(VERSION 3.30)
project(stock1)

set(CMAKE_CXX_STANDARD 23)

add_executable(stock1 main.cpp ./src/Exchange.cpp)
