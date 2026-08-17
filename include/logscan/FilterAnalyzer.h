#pragma once

#include "logscan/LogAnalyzer.h"

#include <optional>
#include <string>

namespace logscan {

// 用户希望从结构化日志中筛选出的条件。
// severity 未设置时不限制级别；contains 为空时不限制正文。
struct FilterCriteria {
    std::optional<Severity> severity;
    std::string contains;
};

// 根据日志级别和正文子串筛选记录。
class FilterAnalyzer final : public LogAnalyzer {
public:
    explicit FilterAnalyzer(FilterCriteria criteria = {});

    std::unique_ptr<LogAnalyzer> clone() const override;

    bool analyze(
        const LogBatch& input,
        BatchReport& output,
        std::string& error) override;

private:
    bool matches(const LogRecord& record) const;

    FilterCriteria criteria_;
};

}  // namespace logscan
