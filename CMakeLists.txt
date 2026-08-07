cmake_minimum_required(VERSION 3.10)
project(logscan LANGUAGES CXX)

option(LOGSCAN_BUILD_TESTS "Build compile-only API tests" ON)
option(LOGSCAN_BUILD_BENCHMARKS "Build benchmark entry points" ON)
option(LOGSCAN_BUILD_TOOLS "Build developer tools" ON)

add_library(logscan STATIC
    src/LogScanner.cpp
    src/ParallelExecutor.cpp
)

target_compile_features(logscan PUBLIC cxx_std_17)

target_include_directories(logscan
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

if(LOGSCAN_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(LOGSCAN_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

if(LOGSCAN_BUILD_TOOLS)
    add_executable(log_generator tools/log_generator.cpp)
    target_link_libraries(log_generator PRIVATE logscan)
    target_compile_features(log_generator PRIVATE cxx_std_17)
endif()
