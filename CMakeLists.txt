cmake_minimum_required(VERSION 3.20)
project(stock LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)
option(BUILD_TESTING "Build tests" ON)

add_subdirectory(external/googletest)


add_subdirectory(src)
add_executable(stock_app main.cpp)
target_link_libraries(stock_app PRIVATE stock_lib)
target_compile_features(stock_app PRIVATE cxx_std_23)


if (BUILD_TESTING)
    add_subdirectory(tests)
endif()
