#include "logscan/FixedFormatLogParser.h"

#include <chrono>
#include <ctime>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace logscan {
namespace {

struct LineFields {
    std::string_view timestamp;
    std::string_view severity;
    std::string_view message;
};

bool split_fields(
    std::string_view text,
    LineFields& output,
    std::string& error)
{
    constexpr std::size_t timestamp_size = 19;

    if (text.size() < timestamp_size + 4) {
        error = "log line is too short";
        return false;
    }

    if (text[19] != ' ' || text[20] != '[') {
        error = "expected '[LEVEL]' after timestamp";
        return false;
    }

    const auto closing_bracket = text.find(']', 21);
    if (closing_bracket == std::string_view::npos) {
        error = "missing closing ']' for severity";
        return false;
    }

    if (closing_bracket + 1 >= text.size() ||
        text[closing_bracket + 1] != ' ') {
        error = "expected a space after severity";
        return false;
    }

    output.timestamp = text.substr(0, timestamp_size);
    output.severity = text.substr(21, closing_bracket - 21);
    output.message = text.substr(closing_bracket + 2);
    return true;
}

bool parse_digits(
    std::string_view text,
    std::size_t offset,
    std::size_t count,
    int& output)
{
    if (offset + count > text.size()) {
        return false;
    }

    int value = 0;
    for (std::size_t index = 0; index < count; ++index) {
        const char character = text[offset + index];
        if (character < '0' || character > '9') {
            return false;
        }
        value = value * 10 + (character - '0');
    }

    output = value;
    return true;
}

bool is_leap_year(int year)
{
    return year % 400 == 0 ||
           (year % 4 == 0 && year % 100 != 0);
}

int days_in_month(int year, int month)
{
    static constexpr int days[] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31,
    };

    if (month < 1 || month > 12) {
        return 0;
    }

    if (month == 2 && is_leap_year(year)) {
        return 29;
    }

    return days[month - 1];
}

bool parse_timestamp(
    std::string_view text,
    std::chrono::system_clock::time_point& output,
    std::string& error)
{
    if (text.size() != 19 ||
        text[4] != '-' ||
        text[7] != '-' ||
        text[10] != ' ' ||
        text[13] != ':' ||
        text[16] != ':') {
        error = "timestamp does not match YYYY-MM-DD HH:MM:SS";
        return false;
    }

    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;

    if (!parse_digits(text, 0, 4, year) ||
        !parse_digits(text, 5, 2, month) ||
        !parse_digits(text, 8, 2, day) ||
        !parse_digits(text, 11, 2, hour) ||
        !parse_digits(text, 14, 2, minute) ||
        !parse_digits(text, 17, 2, second)) {
        error = "timestamp contains non-digit fields";
        return false;
    }

    if (year < 1970 ||
        month < 1 || month > 12 ||
        day < 1 || day > days_in_month(year, month) ||
        hour < 0 || hour > 23 ||
        minute < 0 || minute > 59 ||
        second < 0 || second > 59) {
        error = "timestamp contains an invalid date or time";
        return false;
    }

    std::tm value{};
    value.tm_year = year - 1900;
    value.tm_mon = month - 1;
    value.tm_mday = day;
    value.tm_hour = hour;
    value.tm_min = minute;
    value.tm_sec = second;
    value.tm_isdst = -1;

    const std::time_t timestamp = std::mktime(&value);
    if (timestamp == static_cast<std::time_t>(-1)) {
        error = "timestamp cannot be represented";
        return false;
    }

    output = std::chrono::system_clock::from_time_t(timestamp);
    return true;
}

bool parse_severity(std::string_view text, Severity& output)
{
    if (text == "TRACE") {
        output = Severity::Trace;
    } else if (text == "DEBUG") {
        output = Severity::Debug;
    } else if (text == "INFO") {
        output = Severity::Info;
    } else if (text == "WARNING") {
        output = Severity::Warning;
    } else if (text == "ERROR") {
        output = Severity::Error;
    } else if (text == "CRITICAL") {
        output = Severity::Critical;
    } else {
        return false;
    }

    return true;
}

}  // namespace

std::unique_ptr<LogParser> FixedFormatLogParser::clone() const
{
    return std::make_unique<FixedFormatLogParser>(*this);
}

bool FixedFormatLogParser::parse(
    const RawLogBatch& input,
    LogBatch& output,
    std::string& error)
{
    output = LogBatch{};
    output.id = input.id;
    error.clear();

    output.records.reserve(input.lines.size());

    for (const auto& raw_line : input.lines) {
        LogRecord record;
        std::string line_error;

        if (!parse_line(raw_line, record, line_error)) {
            ParseIssue issue;
            issue.record_id = raw_line.id;
            issue.position = raw_line.position;
            issue.raw_text = raw_line.text;
            issue.error = std::move(line_error);
            output.parse_issues.push_back(std::move(issue));
            continue;
        }

        output.records.push_back(std::move(record));
    }

    return true;
}

bool FixedFormatLogParser::parse_line(
    const RawLogLine& input,
    LogRecord& output,
    std::string& error)
{
    error.clear();

    LineFields fields;
    if (!split_fields(input.text, fields, error)) {
        return false;
    }

    std::chrono::system_clock::time_point timestamp;
    if (!parse_timestamp(fields.timestamp, timestamp, error)) {
        return false;
    }

    Severity severity = Severity::Unknown;
    if (!parse_severity(fields.severity, severity)) {
        error = "unknown severity: " + std::string(fields.severity);
        return false;
    }

    LogRecord record;
    record.id = input.id;
    record.position = input.position;
    record.timestamp = timestamp;
    record.severity = severity;
    record.message = std::string(fields.message);
    record.raw_text = input.text;

    output = std::move(record);
    return true;
}

}  // namespace logscan
