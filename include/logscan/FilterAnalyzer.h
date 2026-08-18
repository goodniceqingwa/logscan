#ifndef FILTERANALYZER_H
#define FILTERANALYZER_H

#include "logscan/LogAnalyzer.h"

#include <optional>
#include <memory>
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
    FilterAnalyzer(FilterCriteria criteria);

    std::unique_ptr<LogAnalyzer> clone() const override;

    bool analyze(const LogBatch& input, BatchReport& output, std::string& error) override;

private:
    FilterCriteria criteria_;

    bool matches(const LogRecord& record) const;
};

}

#endif // FILTERANALYZER_H
