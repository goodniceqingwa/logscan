#ifndef FILTERANALYZER_H
#define FILTERANALYZER_H

#include "logscan/LogAnalyzer.h"

#include <memory>
#include <optional>
#include <string>

namespace logscan {

struct FilterCriteria
{
    std::optional<Severity> severity;
    std::string contains;
};

class FilterAnalyzer final : public LogAnalyzer
{
public:
    explicit FilterAnalyzer(FilterCriteria criteria);

    std::unique_ptr<LogAnalyzer> clone() const override;

    bool analyze(const LogBatch& input, BatchReport& output, std::string& error) override;

private:
    FilterCriteria criteria_;

    bool matches(const LogRecord& record) const;
};

}  // namespace logscan

#endif  // FILTERANALYZER_H
