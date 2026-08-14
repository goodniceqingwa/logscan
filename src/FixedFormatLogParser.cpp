#include "logscan/FixedFormatLogParser.h"

namespace logscan {

std::unique_ptr<logscan::LogParser> FixedFormatLogParser::clone() const
{
    return std::make_unique<FixedFormatLogParser>(*this);
}

bool FixedFormatLogParser::parse(const RawLogBatch& input, LogBatch& output, std::string& error)
{
    output = LogBatch{};
    output.id = input.id;
    error.clear();

    output.records.reserve(input.lines.size());

    for (const auto& raw_line : input.lines)
    {
        LogRecord record;
        std::string line_error;

        if (!parse_line(raw_line, record, line_error))
        {
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

bool FixedFormatLogParser::parse_line(const RawLogLine& input, LogRecord& output, std::string& error)
{
    error.clear();

    // YYYY-MM-DD HH:MM:SS [X]
    if (input.text.size() < 24)
    {
        error = "log line is too short";
        return false;
    }

    // time size is 19
    if (input.text[19] != ' ' || input.text[20] != '[')
    {
        error = "expected '[LEVEL]' after timestamp";
        return false;
    }

    const auto closing_bracket = input.text.find(']', 21);

    if (closing_bracket == std::string::npos)
    {
        error = "missing closing ']' for serverity";
        return false;
    }

    if (closing_bracket + 1 >= input.text.size() || input.text[closing_bracket + 1] != ' ')
    {
        error = "expected a space after severity";
        return false;
    }

    const std::string timestamp_text = input.text.substr(0, 19);
    const std::string severity_text = input.text.substr(21, closing_bracket - 21);
    const std::string message = input.text.substr(closing_bracket + 2);

    Severity severity;

    if (!parse_severity(severity_text, severity))
    {
        error = "unknown severity" + severity_text;
        return false;
    }

    // 传统时间结构
    std::tm parse_time{}; // 初始化
    parse_time.tm_isdst = -1; // 常见时令写法

    std::istringstream timestamp_stream(timestamp_text); // 包装为输入流  ～= cin
    timestamp_stream >> std::get_time(&parse_time, "%Y-%m-%d %H:%M:%S"); // 将字符串转变为固定格式

    if (timestamp_stream.fail())
    {
        error = "invalid timestamp" + timestamp_text;
        return false;
    }

    const int year = parse_time.tm_year + 1900;
    const int month = parse_time.tm_mon + 1;
    const int day = parse_time.tm_mday;
    const int hour = parse_time.tm_hour;
    const int minute = parse_time.tm_min;
    const int second = parse_time.tm_sec;

    if (month < 1 || month > 12 ||
        day < 1 ||
        day > days_in_month(year, month) ||
        hour < 0 || hour > 23 ||
        minute < 0 || minute > 59 ||
        second < 0 || second > 59) {
        error = "invalid timestamp: " + timestamp_text;
        return false;
    }

    const std::time_t timestamp = std::mktime(&parse_time); // mktime 将结构体传转换为time_t 失败返回-1

    if (timestamp == static_cast<std::time_t>(-1))
    {
        error = "timestamp cannot be represented";
        return false;
    }

    output = LogRecord{};
    output.id = input.id;
    output.position = input.position;
    output.timestamp = std::chrono::system_clock::from_time_t(timestamp); // 时间比较 排序 时间差计算
    output.severity = severity;
    output.message = message;
    output.raw_text = input.text;

    return true;
}

bool FixedFormatLogParser::parse_severity(const std::string& text, Severity& output)
{
    if (text == "TRACE")
    {
        output = Severity::Trace;
    }
    else if (text == "DEBUG")
    {
        output = Severity::Debug;
    }
    else if (text == "INFO")
    {
        output = Severity::Info;
    }
    else if (text == "WARNING")
    {
        output = Severity::Warning;
    }
    else if (text == "ERROR")
    {
        output = Severity::Error;
    }
    else if (text == "CRITICAL")
    {
        output = Severity::Critical;
    }
    else
    {
        return false;
    }

    return true;
}

bool FixedFormatLogParser::is_leap_year(int year)
{
    return year % 400 == 0 || (year % 4 == 0 && year % 100 != 0);
}

int FixedFormatLogParser::days_in_month(int year, int month)
{
    static constexpr int days[] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    if (month < 1 || month > 12) {
        return 0;
    }

    if (month == 2 && is_leap_year(year)) {
        return 29;
    }

    return days[month - 1];
}


}