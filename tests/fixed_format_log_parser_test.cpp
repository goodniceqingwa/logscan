#include "logscan/FixedFormatLogParser.h"

#include <iostream>
#include <string>

namespace {

int test_valid_line()
{
    //TODO 无效数据
    return 0;
}

int test_malformed_line_does_not_stop_batch()
{
    logscan::RawLogBatch input;
    input.id = 100;

    logscan::RawLogLine first;
    first.id = 10;
    first.position.source = "mixed.log";
    first.position.line = 1;
    first.text = "2026-08-10 10:00:00 [INFO] Server started";

    logscan::RawLogLine malformed;
    malformed.id = 11;
    malformed.position.source = "mixed.log";
    malformed.position.line = 2;
    malformed.text = "this is not a valid log line";

    logscan::RawLogLine third;
    malformed.id = 12;
    malformed.position.source = "mixed.log";
    malformed.position.line = 3;
    malformed.text = "2026-08-10 10:00:02 [ERROR] Database timeout";

    input.lines.push_back(first);
    input.lines.push_back(malformed);
    input.lines.push_back(third);

    logscan::FixedFormatLogParser parser;
    logscan::LogBatch output;
    std::string error;

    const bool succeeded = parser.parse(input, output, error);

    if (!succeeded) {
        std::cerr
            << "malformed input should not fail the batch: "
            << error << '\n';
        return 20;
    }

    if (!error.empty()) {
        std::cerr
            << "expected no batch-level error, but got: "
            << error << '\n';
        return 21;
    }

    if (output.records.size() != 2) {
        std::cerr
            << "expected 2 valid records, but got "
            << output.records.size() << '\n';
        return 22;
    }

    if (output.parse_issues.size() != 1) {
        std::cerr
            << "expected 1 parse issue, but got "
            << output.parse_issues.size() << '\n';
        return 23;
    }

    if (output.records[0].id != 10 ||
        output.records[1].id != 12) {
        std::cerr
            << "valid records were not preserved in input order\n";
        return 24;
    }

    const auto& issue = output.parse_issues.front();

    if (issue.record_id != 11) {
        std::cerr
            << "expected issue record id 11, but got "
            << issue.record_id << '\n';
        return 25;
    }

    if (issue.position.source != "mixed.log" ||
        issue.position.line != 2) {
        std::cerr << "parse issue position was not preserved\n";
        return 26;
    }

    if (issue.raw_text != malformed.text) {
        std::cerr << "malformed raw text was not preserved\n";
        return 27;
    }

    if (issue.error.empty()) {
        std::cerr << "expected a useful parse error message\n";
        return 28;
    }


    return 0;
}

int test_invalid_calendar_date()
{
    logscan::RawLogBatch input;
    input.id = 101;

    logscan::RawLogLine line;
    line.id = 20;
    line.position.source = "invalid-date.log";
    line.position.line = 1;
    line.text = "2026-02-31 10:00:01 [ERROR] Impossible date";

    input.lines.push_back(line);

    logscan::FixedFormatLogParser parser;
    logscan::LogBatch output;
    std::string error;

    const bool succeeded = parser.parse(input, output, error);

    if (!succeeded) {
        std::cerr
            << "invalid date should be a line-level issue\n";
        return 30;
    }

    if (!output.records.empty()) {
        std::cerr
            << "invalid calendar date produced a record\n";
        return 31;
    }

    if (output.parse_issues.size() != 1) {
        std::cerr
            << "expected one issue for invalid date, but got "
            << output.parse_issues.size() << '\n';
        return 32;
    }

    return 0;
}

} // namespace


int main ()
{
    if (const int result = test_valid_line(); result != 0)
    {
        return result;
    }

    if (const int result = test_malformed_line_does_not_stop_batch(); result != 0)
    {
        return result;
    }

    if (const int result = test_invalid_calendar_date())
    {
        return result;
    }
    return 0;


}