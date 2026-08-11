#include "logscan/FileLogSource.h"

#include <iostream>
#include <string>

int main (int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "usage: file_log_source_test <fixture-path>\n";
        return 1;
    }

    logscan::FileLogSource source{2};
    logscan::ScanRequest request;
    request.inputs.emplace_back(argv[1]);

    std::string error;

    const bool opened = source.open(request, error);

    if (!opened)
    {
        std::cerr << "expected fixture file to open successfully , but got:" << error << '\n';
        return 2;
    }

    if (!error.empty())
    {
        std::cerr << "expected empty error after successful open, but got: " << error << '\n';
        return 3;
    }

    logscan::RawLogBatch batch;

    // 第一次读取
    const auto first_read = source.read(batch);

    if (first_read.status != logscan::SourceReadStatus::BatchReady)
    {
        std::cerr << "expected first read to return BatchReadu, but got " << static_cast<int>(first_read.status) << '\n';
        return 4;
    }

    if (batch.id != 0)
    {
        std::cerr << "expected first batch id to be 0, but got" << batch.id  << '\n';
        return 5;
    }

    if (batch.lines.size() != 2)
    {
        std::cerr << " expected first batch to contain 2 lines, but got " << batch.lines.size() << '\n';
        return 6;
    }

    if (batch.lines[0].id != 0 || batch.lines[1].id != 1)
    {
        std::cerr << " expected first batch record ids to be 0 and 1 \n";
        return 7;
    }

    if (batch.lines[0].position.line != 1 || batch.lines[1].position.line != 2)
    {
        std::cerr << " expected first batch line numbers to be 1 and 2 \n";
        return 8;
    }

    if (batch.lines[0].text != "2026-08-10 10:00:00 [INFO] Server started")
    {
        std::cerr << " unexpected text in first log line: " << batch.lines[0].text << '\n';
        return 9;
    }

    if (batch.lines[1].text !=
        "2026-08-10 10:00:01 [ERROR] Database timeout") {
        std::cerr
            << "unexpected text in second log line: "
            << batch.lines[1].text
            << '\n';
        return 10;
    }

    // 第二次读取
    const auto second_read = source.read(batch);

    if (second_read.status != logscan::SourceReadStatus::BatchReady)
    {
        std::cerr << "expected second read to return BatchReady, but got "<< static_cast<int>(second_read.status) << '\n';
        return 11;
    }

    if (batch.id != 1)
    {
        std::cerr << "expected second batch id to be 1, but got " << batch.id << '\n';
        return 12;
    }

    if (batch.lines.size() != 1)
    {
        std::cerr << "expectde second batch to contain 1 line, but got " << batch.lines.size() << '\n';
        return 13;
    }

    if (batch.lines[0].id != 2) {
        std::cerr
            << "expected final record id to be 2, but got "
            << batch.lines[0].id
            << '\n';
        return 14;
    }

    if (batch.lines[0].position.line != 3) {
        std::cerr
            << "expected final line number to be 3, but got "
            << batch.lines[0].position.line
            << '\n';
        return 15;
    }

    if (batch.lines[0].text !=
        "2026-08-10 10:00:02 [WARNING] Request is slow") {
        std::cerr
            << "unexpected text in final log line: "
            << batch.lines[0].text
            << '\n';
        return 16;
    }

    // 第三次读取：所有数据已经消费完，预期到达 EOF。
    const auto third_read = source.read(batch);

    if (third_read.status != logscan::SourceReadStatus::EndOfInput) {
        std::cerr
            << "expected third read to return EndOfInput, but got "
            << static_cast<int>(third_read.status)
            << '\n';
        return 17;
    }

    // read() 应该清空调用方传入的旧 batch，
    // 不能让上一次读取的内容残留。
    if (!batch.lines.empty()) {
        std::cerr
            << "expected batch to be empty after EndOfInput\n";
        return 18;
    }

    source.close();

    // close() 后继续读取应该返回 Error，而不是崩溃或返回旧数据。
    const auto read_after_close = source.read(batch);

    if (read_after_close.status !=
        logscan::SourceReadStatus::Error) {
        std::cerr
            << "expected read after close to return Error, but got "
            << static_cast<int>(read_after_close.status)
            << '\n';
        return 19;
    }

    return 0;


}
