#ifndef FIXEDFORMATLOGPARSER_H
#define FIXEDFORMATLOGPARSER_H

#include "logscan/LogParser.h"

#include <memory>
#include <string>

namespace logscan {

class FixedFormatLogParser final : public LogParser {
public:
    std::unique_ptr<LogParser> clone() const override;

    bool parse (const RawLogBatch& input, LogBatch& output, std::string& error) override;

private:
    static bool parse_line(const RawLogLine& input, LogRecord& output, std::string& error);

    static bool parse_severity(const std::string& text, Severity& output);

    static bool is_leap_year(int year);

    static int days_in_month(int year, int month);

};

}

#endif // FIXEDFORMATLOGPARSER_H
