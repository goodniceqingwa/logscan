#include "logscan/FilterAnalyzer.h"

#include <iostream>
#include <string>
#include <utility>

namespace {

logscan::LogRecord make_record (
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
    input.id = 42;
    input.worker_id = 3;

    input.records.push_back(make_record(100, logscan::Severity::Warning, "slow request"));

    input.records.push_back(make_record(101, logscan::Severity::Error, "database timeout"));

    logscan::FilterCriteria criteria;
    criteria.severity = logscan::Severity::Error;

    logscan::FilterAnalyzer analyzer(std::move(criteria));

    logscan::BatchReport output;
    std::string error;

    const bool succeeded = analyzer.analyze(input, output, error);

    if (!succeeded)
    {
        std::cerr << "expected analysis to succeed, but got:" << error << '\n';
        return 1;
    }

    if (!error.empty())
    {
        std::cerr << "expected an empty error message \n";
        return 2;
    }

    // 验证批次信息被正确传递。
    if (output.batch_id != 42 ||
        output.worker_id != 3 ||
        output.records_analyzed != 2) {
        std::cerr << "unexpected batch report metadata\n";
        return 3;
    }

    // WARNING 不应该匹配，因此只能得到一个 Finding。
    if (output.findings.size() != 1) {
        std::cerr
            << "expected one finding, but got "
            << output.findings.size() << '\n';
        return 4;
    }

    // 得到的 Finding 必须来自 ERROR 那条记录。
    const auto& finding = output.findings.front();

    if (finding.record_id != 101) {
        std::cerr << "the ERROR record was not selected\n";
        return 5;
    }

    if (finding.severity != logscan::Severity::Error) {
        std::cerr << "finding severity was not preserved\n";
        return 6;
    }

    if (finding.message != "database timeout") {
        std::cerr << "finding message was not preserved\n";
        return 7;
    }

    return 0;
}

int test_contains_filter()
{
    logscan::LogBatch input;

    input.records.push_back(make_record(200, logscan::Severity::Error, "database timeout"));
    input.records.push_back(make_record(201, logscan::Severity::Error, "connection refused"));
    input.records.push_back(make_record(202, logscan::Severity::Error, "Database Timeout"));

    logscan::FilterCriteria criteria;
    criteria.contains = "timeout";

    logscan::FilterAnalyzer analyzer(std::move(criteria));

    logscan::BatchReport output;
    std::string error;

    const bool succeeded = analyzer.analyze(input, output, error);

    if (!succeeded)
    {
        std::cerr << "contains analysis failed:" << error << '\n';
        return 10;
    }

    if (!error.empty())
    {
        std::cerr<<"expected an empty error message\n";
        return 11;
    }

    if (output.findings.size() != 1)
    {
        std::cerr << "expected one finding, but got " << output.findings.size() << '\n';
        return 12;
    }

    if (output.findings.front().record_id != 200) {
        std::cerr << "the wrong record was selected\n";
        return 13;
    }

    return 0;


}

}

int main()
{
    if (const int result = test_exact_severity_filter();
        result != 0) {
        return result;
    }

    return test_contains_filter();
}