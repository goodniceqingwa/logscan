#include "logscan/FilterAnalyzer.h"

#include <iostream>
#include <memory>
#include <string>
#include <utility>

namespace {

logscan::LogRecord make_record(
    logscan::RecordId id,
    logscan::Severity severity,
    std::string message)
{
    logscan::LogRecord record;
    record.id = id;
    record.position.source = "app.log";
    record.position.line = id;
    record.position.byte_offset = id * 10;
    record.severity = severity;
    record.message = std::move(message);
    return record;
}

int test_exact_severity_filter()
{
    logscan::LogBatch input;
    input.id = 10;
    input.worker_id = 2;
    input.records.push_back(
        make_record(1, logscan::Severity::Warning, "slow request"));
    input.records.push_back(
        make_record(2, logscan::Severity::Error, "request failed"));

    logscan::FilterCriteria criteria;
    criteria.severity = logscan::Severity::Error;
    logscan::FilterAnalyzer analyzer(std::move(criteria));

    logscan::BatchReport output;
    std::string error;
    if (!analyzer.analyze(input, output, error)) {
        std::cerr << "severity analysis failed: " << error << '\n';
        return 1;
    }

    if (!error.empty() || output.batch_id != 10 ||
        output.worker_id != 2 || output.records_analyzed != 2 ||
        output.findings.size() != 1 ||
        output.findings.front().record_id != 2) {
        std::cerr << "severity filter produced an unexpected report\n";
        return 2;
    }

    return 0;
}

int test_contains_filter_is_case_sensitive()
{
    logscan::LogBatch input;
    input.records.push_back(
        make_record(3, logscan::Severity::Error, "database timeout"));
    input.records.push_back(
        make_record(4, logscan::Severity::Error, "Database Timeout"));
    input.records.push_back(
        make_record(5, logscan::Severity::Error, "connection refused"));

    logscan::FilterCriteria criteria;
    criteria.contains = "timeout";
    logscan::FilterAnalyzer analyzer(std::move(criteria));

    logscan::BatchReport output;
    std::string error;
    if (!analyzer.analyze(input, output, error)) {
        std::cerr << "contains analysis failed: " << error << '\n';
        return 10;
    }

    if (output.findings.size() != 1 ||
        output.findings.front().record_id != 3) {
        std::cerr << "contains filter must use a case-sensitive substring\n";
        return 11;
    }

    return 0;
}

int test_combined_filters_use_and()
{
    logscan::LogBatch input;
    input.records.push_back(
        make_record(6, logscan::Severity::Error, "database timeout"));
    input.records.push_back(
        make_record(7, logscan::Severity::Info, "database timeout"));
    input.records.push_back(
        make_record(8, logscan::Severity::Error, "connection refused"));

    logscan::FilterCriteria criteria;
    criteria.severity = logscan::Severity::Error;
    criteria.contains = "timeout";
    logscan::FilterAnalyzer analyzer(std::move(criteria));

    logscan::BatchReport output;
    std::string error;
    if (!analyzer.analyze(input, output, error)) {
        std::cerr << "combined analysis failed: " << error << '\n';
        return 20;
    }

    if (output.findings.size() != 1) {
        std::cerr << "combined filters must require every condition\n";
        return 21;
    }

    const auto& finding = output.findings.front();
    if (finding.record_id != 6 ||
        finding.position.source != "app.log" ||
        finding.position.line != 6 ||
        finding.position.byte_offset != 60 ||
        finding.severity != logscan::Severity::Error ||
        finding.rule_id != "filter.match" ||
        finding.message != "database timeout") {
        std::cerr << "finding did not preserve record metadata\n";
        return 22;
    }

    return 0;
}

int test_empty_criteria_matches_all_and_resets_output()
{
    logscan::LogBatch input;
    input.id = 30;
    input.worker_id = 4;
    input.records.push_back(
        make_record(9, logscan::Severity::Info, "server started"));
    input.records.push_back(
        make_record(10, logscan::Severity::Critical, "disk full"));

    logscan::FilterAnalyzer analyzer;
    logscan::BatchReport output;
    output.batch_id = 999;
    output.records_analyzed = 999;
    output.findings.push_back(logscan::Finding{});

    std::string error = "stale error";
    if (!analyzer.analyze(input, output, error)) {
        std::cerr << "unfiltered analysis failed: " << error << '\n';
        return 30;
    }

    if (!error.empty() || output.batch_id != 30 ||
        output.worker_id != 4 || output.records_analyzed != 2 ||
        output.findings.size() != 2 ||
        output.findings[0].record_id != 9 ||
        output.findings[1].record_id != 10) {
        std::cerr << "empty criteria did not match all records or reset output\n";
        return 31;
    }

    return 0;
}

int test_clone_preserves_criteria()
{
    logscan::FilterCriteria criteria;
    criteria.severity = logscan::Severity::Critical;
    criteria.contains = "disk";
    logscan::FilterAnalyzer prototype(std::move(criteria));
    std::unique_ptr<logscan::LogAnalyzer> analyzer = prototype.clone();

    logscan::LogBatch input;
    input.records.push_back(
        make_record(11, logscan::Severity::Critical, "disk full"));
    input.records.push_back(
        make_record(12, logscan::Severity::Error, "disk full"));

    logscan::BatchReport output;
    std::string error;
    if (!analyzer || !analyzer->analyze(input, output, error)) {
        std::cerr << "cloned analyzer failed\n";
        return 40;
    }

    if (output.findings.size() != 1 ||
        output.findings.front().record_id != 11) {
        std::cerr << "clone did not preserve filter criteria\n";
        return 41;
    }

    return 0;
}

}  // namespace

int main()
{
    if (const int result = test_exact_severity_filter(); result != 0) {
        return result;
    }
    if (const int result = test_contains_filter_is_case_sensitive(); result != 0) {
        return result;
    }
    if (const int result = test_combined_filters_use_and(); result != 0) {
        return result;
    }
    if (const int result = test_empty_criteria_matches_all_and_resets_output();
        result != 0) {
        return result;
    }
    if (const int result = test_clone_preserves_criteria(); result != 0) {
        return result;
    }
    return 0;
}
