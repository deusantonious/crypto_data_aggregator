cmake_minimum_required(VERSION 3.15)
project(crypto_data_aggregator LANGUAGES CXX)

set(PROJECT_TARGET_NAME ${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_CLANG_TIDY "Enable clang-tidy during compilation" OFF)

if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE
        NAMES clang-tidy clang-tidy-19 clang-tidy-18 clang-tidy-17
    )

    if(CLANG_TIDY_EXE)
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-p=${CMAKE_BINARY_DIR};--use-color;--extra-arg-before=--driver-mode=g++;--extra-arg=-std=gnu++20")
        message(STATUS "clang-tidy enabled: ${CLANG_TIDY_EXE}")
    else()
        message(WARNING "ENABLE_CLANG_TIDY is ON, but clang-tidy was not found. Continuing without clang-tidy.")
    endif()
endif()



find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)

option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)
    include(GoogleTest)
endif()

add_subdirectory(src/market)
add_subdirectory(src/aggregation)
add_subdirectory(src/output)

set(PROJECT_HEADERS
    src/app/application.hpp
    src/app/config_loader.hpp
    src/app/snapshot_output_adapter.hpp
)

set(PROJECT_SOURCES
    src/main.cpp
    src/app/application.cpp
    src/app/config_loader.cpp
    src/app/snapshot_output_adapter.cpp
)

add_executable(${PROJECT_TARGET_NAME}
    ${PROJECT_HEADERS}
    ${PROJECT_SOURCES}
)

target_include_directories(${PROJECT_TARGET_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_TARGET_NAME} PRIVATE
    output
    aggregation
    nlohmann_json::nlohmann_json
    spdlog::spdlog
)

if(BUILD_TESTS)
    add_executable(app_tests
        src/app/tests/config_loader_test.cpp
        src/app/tests/snapshot_output_adapter_test.cpp
        src/app/config_loader.cpp
        src/app/snapshot_output_adapter.cpp
    )
    target_include_directories(app_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )
    target_link_libraries(app_tests PRIVATE
        output
        aggregation
        nlohmann_json::nlohmann_json
        GTest::gtest_main
    )
    gtest_discover_tests(app_tests)
endif()
