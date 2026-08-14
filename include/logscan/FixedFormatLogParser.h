#pragma once

#include "logscan/LogParser.h"

#include <memory>
#include <string>

namespace logscan {

class FixedFormatLogParser final : public LogParser {
public:
    std::unique_ptr<LogParser> clone() const override;

    bool parse(
        const RawLogBatch& input,
        LogBatch& output,
        std::string& error) override;

private:
    static bool parse_line(
        const RawLogLine& input,
        LogRecord& output,
        std::string& error);
};

}  // namespace logscan
