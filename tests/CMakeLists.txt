# 该测试是 API smoke test，不依赖具体日志格式或并发实现。
add_executable(logscan_api_test
    api_compile_test.cpp
)

# 链接框架目标，确保公共头文件和目标声明可正常使用。
target_link_libraries(logscan_api_test PRIVATE logscan)
target_compile_features(logscan_api_test PRIVATE cxx_std_17)

add_test(NAME logscan_api_test COMMAND logscan_api_test)
