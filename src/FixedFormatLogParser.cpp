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

    std::tm parse_time{};
    parse_time.tm_isdst = -1;

    std::istringstream timestamp_stream(timestamp_text);
    timestamp_stream >> std::get_time(&parse_time, "%Y-%m-%d %H:%M:%S");

    if (timestamp_stream.fail())
    {
        error = "invalid timestamp" + timestamp_text;
        return false;
    }

    const std::time_t timestamp = std::mktime(&parse_time);

    if (timestamp == static_cast<std::time_t>(-1))
    {
        error = "timestamp cannot be represented";
        return false;
    }

    output = LogRecord{};
    output.id = input.id;
    output.position = input.position;
    output.timestamp = std::chrono::system_clock::from_time_t(timestamp);
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


}