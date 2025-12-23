cmake_minimum_required(VERSION 3.20)
project(stock_test CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(stock_lib INTERFACE)
target_include_directories(stock_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../src)

if (BUILD_TESTING)
    add_executable(stock_test
            test.cpp
            ../src/Exchange.cpp
    )
    target_link_libraries(stock_test PRIVATE stock_lib GTest::gtest GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(stock_test)
endif ()
