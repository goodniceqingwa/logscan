# 当前只编译基准入口，不引入第三方 benchmark 框架。
add_executable(logscan_benchmark
    logscan_benchmark.cpp
)

# 后续基准可直接复用 logscan 公共接口。
target_link_libraries(logscan_benchmark PRIVATE logscan)
target_compile_features(logscan_benchmark PRIVATE cxx_std_17)
