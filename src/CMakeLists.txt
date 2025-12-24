add_library(stock_lib
        Exchange.cpp
        OrderBook.cpp
        Broker.cpp
)

target_compile_features(stock_lib PUBLIC cxx_std_23)
target_include_directories(stock_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
