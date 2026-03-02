set(TARGET_NAME output)

set(HEADERS
    include/output/i_snapshot_printer.hpp
    include/output/snapshot_printer_factory.hpp

    src/interfaces/i_output_stream.hpp
    src/interfaces/i_snapshot_formatter.hpp

    src/file_output_stream.hpp
    src/formatted_snapshot_printer.hpp
    src/plain_text_formatter.hpp
)

set(SOURCES
    src/plain_text_formatter.cpp
    src/file_output_stream.cpp
    src/snapshot_printer_factory.cpp
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
        ${CMAKE_CURRENT_SOURCE_DIR}/src/interfaces
)

target_link_libraries(${TARGET_NAME}
    PUBLIC
        spdlog::spdlog
)
if(BUILD_TESTS)
    add_executable(output_tests
        tests/plain_text_formatter_test.cpp
        tests/formatted_snapshot_printer_test.cpp
        tests/file_output_stream_test.cpp
        tests/snapshot_printer_factory_test.cpp
    )
    target_include_directories(output_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )
    target_link_libraries(output_tests PRIVATE
        output
        GTest::gtest_main
        GTest::gmock
    )
    gtest_discover_tests(output_tests)
endif()