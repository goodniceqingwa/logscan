#include "logscan/FixedFormatLogParser.h"

#include <iostream>
#include <string>

int main ()
{
    logscan::RawLogBatch input;
    input.id = 42;

    logscan::RawLogLine line;
    line.id = 7;
    line.position.source = "app.log";
    line.position.line = 13;
    line.position.byte_offset = 128;
    line.text = "2026-08-10 10:00:01 [ERROR] Database timeout";

    input.lines.push_back(line);

    logscan::FixedFormatLogParser parser;
    logscan::LogBatch output;
    std::string error;

    const bool succeeded = parser.parse(input, output, error);

    if (!succeeded)
    {
        std::cerr << "expected parsing to succeed, but got " << error << '\n';
        return 1;
    }

    if (!error.empty())
    {
        std::cerr << "expected empty batch error, but got" << error << '\n';
        return 2;
    }

    if (output.id != 42)
    {
        std::cerr << "expected output batch id 42, but got " << output.id << '\n';
        return 3;
    }

    if (output.records.size() != 1) {
        std::cerr
            << "expected one parsed record, but got "
            << output.records.size() << '\n';
        return 4;
    }

    if (!output.parse_issues.empty()) {
        std::cerr
            << "expected no parse issues, but got "
            << output.parse_issues.size() << '\n';
        return 5;
    }

    const auto& record = output.records.front();

    if (record.id != 7) {
        std::cerr
            << "expected record id 7, but got "
            << record.id << '\n';
        return 6;
    }

    if (record.position.source != "app.log" ||
        record.position.line != 13 ||
        record.position.byte_offset != 128) {
        std::cerr << "source position was not preserved\n";
        return 7;
    }

    if (record.severity != logscan::Severity::Error) {
        std::cerr
            << "expected Error severity, but got "
            << static_cast<int>(record.severity) << '\n';
        return 8;
    }

    if (record.message != "Database timeout") {
        std::cerr
            << "unexpected message: "
            << record.message << '\n';
        return 9;
    }

    if (record.raw_text != line.text) {
        std::cerr << "original log text was not preserved\n";
        return 10;
    }

    return 0;


}