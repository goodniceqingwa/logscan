#include "logscan/FixedFormatLogParser.h"

#include <iostream>
#include <string>

namespace {

int test_valid_line()
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

    if (!parser.parse(input, output, error)) {
        std::cerr << "expected parsing to succeed, but got: " << error << '\n';
        return 1;
    }

    if (!error.empty() || output.records.size() != 1 ||
        !output.parse_issues.empty()) {
        std::cerr << "valid input produced an unexpected batch result\n";
        return 2;
    }

    const auto& record = output.records.front();
    if (output.id != 42 || record.id != 7 ||
        record.position.source != "app.log" ||
        record.position.line != 13 ||
        record.position.byte_offset != 128) {
        std::cerr << "record identity or source position was not preserved\n";
        return 3;
    }

    if (record.severity != logscan::Severity::Error ||
        record.message != "Database timeout" ||
        record.raw_text != line.text) {
        std::cerr << "valid log fields were parsed incorrectly\n";
        return 4;
    }

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
    third.id = 12;
    third.position.source = "mixed.log";
    third.position.line = 3;
    third.text = "2026-08-10 10:00:02 [ERROR] Database timeout";

    input.lines.push_back(first);
    input.lines.push_back(malformed);
    input.lines.push_back(third);

    logscan::FixedFormatLogParser parser;
    logscan::LogBatch output;
    std::string error;

    if (!parser.parse(input, output, error)) {
        std::cerr << "malformed input should not fail the batch: " << error << '\n';
        return 10;
    }

    if (!error.empty() || output.records.size() != 2 ||
        output.parse_issues.size() != 1) {
        std::cerr << "mixed input produced unexpected record or issue counts\n";
        return 11;
    }

    if (output.records[0].id != 10 || output.records[1].id != 12) {
        std::cerr << "valid records were not preserved in input order\n";
        return 12;
    }

    const auto& issue = output.parse_issues.front();
    if (issue.record_id != 11 ||
        issue.position.source != "mixed.log" ||
        issue.position.line != 2 ||
        issue.raw_text != malformed.text ||
        issue.error.empty()) {
        std::cerr << "malformed record diagnostic was not preserved\n";
        return 13;
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

    if (!parser.parse(input, output, error)) {
        std::cerr << "invalid date should be a line-level issue\n";
        return 20;
    }

    if (!output.records.empty() || output.parse_issues.size() != 1) {
        std::cerr << "invalid date was not reported as one parse issue\n";
        return 21;
    }

    return 0;
}

int test_unknown_severity()
{
    logscan::RawLogBatch input;
    input.id = 102;

    logscan::RawLogLine line;
    line.id = 21;
    line.text = "2026-08-10 10:00:01 [FATAL] Unsupported severity";
    input.lines.push_back(line);

    logscan::FixedFormatLogParser parser;
    logscan::LogBatch output;
    std::string error;
    if (!parser.parse(input, output, error)) {
        std::cerr << "unknown severity should be a line-level issue\n";
        return 30;
    }

    if (!output.records.empty() || output.parse_issues.size() != 1 ||
        output.parse_issues.front().error.find("FATAL") == std::string::npos) {
        std::cerr << "unknown severity diagnostic is incomplete\n";
        return 31;
    }

    return 0;
}

}  // namespace

int main()
{
    if (const int result = test_valid_line(); result != 0) {
        return result;
    }
    if (const int result = test_malformed_line_does_not_stop_batch(); result != 0) {
        return result;
    }
    if (const int result = test_invalid_calendar_date(); result != 0) {
        return result;
    }
    if (const int result = test_unknown_severity(); result != 0) {
        return result;
    }
    return 0;
}
