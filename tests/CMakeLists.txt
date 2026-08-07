add_executable(logscan_api_test
    api_compile_test.cpp
)

target_link_libraries(logscan_api_test PRIVATE logscan)
target_compile_features(logscan_api_test PRIVATE cxx_std_17)

add_test(NAME logscan_api_test COMMAND logscan_api_test)
