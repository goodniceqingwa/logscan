add_executable(logscan_benchmark
    logscan_benchmark.cpp
)

target_link_libraries(logscan_benchmark PRIVATE logscan)
target_compile_features(logscan_benchmark PRIVATE cxx_std_17)
