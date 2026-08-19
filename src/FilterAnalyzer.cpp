#include "logscan/FilterAnalyzer.h"

#include <memory>
#include <utility>

namespace logscan {

FilterAnalyzer::FilterAnalyzer(FilterCriteria criteria)
    : criteria_(std::move(criteria))
{
}

std::unique_ptr<LogAnalyzer> FilterAnalyzer::clone() const
{
    return std::make_unique<FilterAnalyzer>(*this);
}

bool FilterAnalyzer::analyze(
    const LogBatch& input,
    BatchReport& output,
    std::string& error)
{
    error.clear();

    BatchReport report;
    report.batch_id = input.id;
    report.worker_id = input.worker_id;
    report.records_analyzed = input.records.size();

    report.findings.reserve(input.records.size());

    for (const auto& record : input.records)
    {
        if (!matches(record))
        {
            continue;
        }

        Finding finding;
        finding.record_id = record.id;
        finding.position = record.position;
        finding.severity = record.severity;
        finding.rule_id = "filter.match";
        finding.message = record.message;

        report.findings.push_back(std::move(finding));
    }

    output = std::move(report);

    return true;
}

bool FilterAnalyzer::matches(const LogRecord& record) const
{
    if (criteria_.severity.has_value() && record.severity != *criteria_.severity)
    {
        return false;
    }

    if (!criteria_.contains.empty() && record.message.find(criteria_.contains) == std::string::npos)
    {
        return false;
    }

    return true;
}

}  // namespace logscan
