add_executable(logscan_api_test
    api_compile_test.cpp
)

target_link_libraries(logscan_api_test
    PRIVATE
        logscan
)

target_compile_features(logscan_api_test
    PRIVATE
        cxx_std_17
)

add_test(
    NAME logscan_api_test
    COMMAND logscan_api_test
)


add_executable(file_log_source_test
    file_log_source_test.cpp
)

target_link_libraries(file_log_source_test
    PRIVATE
        logscan
)

target_compile_features(file_log_source_test
    PRIVATE
        cxx_std_17
)

add_test(
    NAME file_log_source_test
    COMMAND
        file_log_source_test
        "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/basic.log"
)
