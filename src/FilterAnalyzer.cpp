#include "logscan/FilterAnalyzer.h"

#include <memory>
#include <utility>

namespace logscan
{
FilterAnalyzer::FilterAnalyzer(FilterCriteria criteria) : criteria_(std::move(criteria))
{

}

std::unique_ptr<LogAnalyzer> FilterAnalyzer::clone() const
{
    return std::make_unique<FilterAnalyzer>(*this);
}

bool FilterAnalyzer::analyze(const LogBatch& input, BatchReport& output, std::string& error)
{
    (void)input;
    (void)output;
    error = "null";

    return false;
}

bool FilterAnalyzer::matches(const LogRecord& record) const
{
    (void)record;

    return false;
}


}
