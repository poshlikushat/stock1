cmake_minimum_required(VERSION 3.20)
project(stock CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)
option(BUILD_TESTING "Build tests" ON)

# это важно: подключаем googletest до tests
add_subdirectory(external/googletest ${CMAKE_BINARY_DIR}/external/googletest)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif()
