set(TARGET_NAME market)

find_package(Boost REQUIRED CONFIG)
find_package(OpenSSL REQUIRED)

set(HEADERS
    include/market/i_market_stream_handler.hpp
    include/market/market_streams_controller.hpp
    include/market/trade_event.hpp

    src/market_stream_handler.hpp
    src/binance/binance_connection_manager.hpp
    src/binance/binance_trade_message_parser.hpp

    src/binance/binance_config.hpp
    src/interfaces/i_exchange_connection_manager.hpp
)

set(SOURCES
    src/binance/binance_connection_manager.cpp
    src/market_stream_handler.cpp
    src/market_streams_controller.cpp
)

add_library(${TARGET_NAME} STATIC
    ${HEADERS}
    ${SOURCES}
)

target_include_directories(${TARGET_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${TARGET_NAME}
    PRIVATE
        Boost::headers
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        OpenSSL::SSL
        OpenSSL::Crypto
)

if(BUILD_TESTS)
    add_executable(market_tests
        tests/market_stream_handler_test.cpp
        tests/binance_trade_message_parser_test.cpp
        tests/binance_config_test.cpp
        tests/binance_connection_manager_test.cpp
        tests/market_streams_controller_test.cpp
    )
    target_include_directories(market_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )
    target_link_libraries(market_tests PRIVATE
        market
        nlohmann_json::nlohmann_json
        Boost::headers
        OpenSSL::SSL
        OpenSSL::Crypto
        GTest::gtest_main
        GTest::gmock
    )
    gtest_discover_tests(market_tests)
endif()
