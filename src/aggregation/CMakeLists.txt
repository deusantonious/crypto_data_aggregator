set(TARGET_NAME aggregation)

set(HEADERS
    include/aggregation/i_statistics_aggregator.hpp
    include/aggregation/symbol_snapshot.hpp

    src/statistics_aggregator.hpp
    src/window_stats.hpp
)

set(SOURCES
    src/window_stats.cpp
    src/statistics_aggregator.cpp
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
    PUBLIC
        market
)

if(BUILD_TESTS)
    add_executable(aggregation_tests
        tests/window_stats_test.cpp
        tests/statistics_aggregator_test.cpp
    )
    target_include_directories(aggregation_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    target_link_libraries(aggregation_tests PRIVATE
        aggregation
        GTest::gtest_main
    )
    gtest_discover_tests(aggregation_tests)
endif()
