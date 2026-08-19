cmake_minimum_required(VERSION 3.10)
project(logscan LANGUAGES CXX)

# 构建开关：测试需要显式开启，默认只构建库和开发工具。
option(LOGSCAN_BUILD_TESTS "Build logscan tests" ON)
option(LOGSCAN_BUILD_BENCHMARKS "Build benchmark entry points" ON)
option(LOGSCAN_BUILD_TOOLS "Build developer tools" ON)

# 核心静态库。
add_library(logscan STATIC
    src/LogScanner.cpp
    src/ParallelExecutor.cpp
    include/logscan/LogAnalyzer.h
    include/logscan/FilterAnalyzer.h
    include/logscan/LogParser.h
    include/logscan/LogReportSink.h
    include/logscan/ConsoleReportSink.h
    include/logscan/LogScanner.h
    include/logscan/LogScannerConfig.h
    include/logscan/LogSource.h
    include/logscan/LogTypes.h
    include/logscan/FileLogSource.h
    src/FileLogSource.cpp
    include/logscan/FixedFormatLogParser.h
    src/FixedFormatLogParser.cpp
    src/FilterAnalyzer.cpp
    src/ConsoleReportSink.cpp
)

target_compile_features(logscan PUBLIC cxx_std_17)

# include/ 对库的使用者可见，src/ 仅属于库内部实现。
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
    # 基准入口独立编译，后续可替换为实际 benchmark harness。
    add_subdirectory(benchmarks)
endif()

if(LOGSCAN_BUILD_TOOLS)
    # 日志生成器是开发辅助工具，不属于核心库 API。
    add_executable(log_generator tools/log_generator.cpp)
    target_link_libraries(log_generator PRIVATE logscan)
    target_compile_features(log_generator PRIVATE cxx_std_17)
endif()
