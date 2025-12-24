add_executable(stock_test
        test.cpp
)

target_link_libraries(stock_test
        PRIVATE
        stock_lib
        GTest::gtest
        GTest::gtest_main
)

target_compile_features(stock_test PRIVATE cxx_std_23)

include(GoogleTest)
gtest_discover_tests(stock_test)
