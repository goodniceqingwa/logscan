#include "logscan/ConsoleReportSink.h"

#include <iostream>
#include <sstream>
#include <string>
#include <utility>

namespace {

logscan::Finding make_finding(
    logscan::RecordId id,
    std::string source,
    std::uint64_t line,
    logscan::Severity severity,
    std::string message)
{
    logscan::Finding finding;
    finding.record_id = id;
    finding.position.source = std::move(source);
    finding.position.line = line;
    finding.severity = severity;
    finding.rule_id = "filter.match";
    finding.message = std::move(message);
    return finding;
}

int test_writes_findings_and_summary()
{
    std::ostringstream output;
    logscan::ConsoleReportSink sink(output);

    logscan::ScanRequest request;
    request.inputs.push_back("app.log");

    std::string error;
    if (!sink.begin(request, error) || !error.empty()) {
        std::cerr << "sink failed to begin: " << error << '\n';
        return 1;
    }

    logscan::BatchReport batch;
    batch.batch_id = 1;
    batch.worker_id = 0;
    batch.records_analyzed = 2;
    batch.findings.push_back(
        make_finding(
            7,
            "app.log",
            7,
            logscan::Severity::Error,
            "database timeout"));
    batch.findings.push_back(
        make_finding(
            9,
            "worker.log",
            9,
            logscan::Severity::Warning,
            "slow request"));

    if (!sink.consume(std::move(batch), error) || !error.empty()) {
        std::cerr << "sink failed to consume: " << error << '\n';
        return 2;
    }

    logscan::ScanReport report;
    report.statistics.batches_read = 2;
    report.statistics.records_read = 12;
    report.statistics.records_analyzed = 11;
    report.statistics.findings_emitted = 2;

    if (!sink.end(report, error) || !error.empty()) {
        std::cerr << "sink failed to end: " << error << '\n';
        return 3;
    }

    const std::string expected =
        "app.log:7 [ERROR] database timeout\n"
        "worker.log:9 [WARNING] slow request\n"
        "summary: batches=2 records=12 analyzed=11 findings=2\n";

    if (output.str() != expected) {
        std::cerr
            << "unexpected console output:\n"
            << output.str();
        return 4;
    }

    return 0;
}

int test_rejects_invalid_lifecycle()
{
    std::ostringstream output;
    logscan::ConsoleReportSink sink(output);
    std::string error;

    logscan::BatchReport batch;
    if (sink.consume(std::move(batch), error) || error.empty()) {
        std::cerr << "consume before begin should fail\n";
        return 10;
    }

    logscan::ScanReport report;
    if (sink.end(report, error) || error.empty()) {
        std::cerr << "end before begin should fail\n";
        return 11;
    }

    logscan::ScanRequest request;
    if (!sink.begin(request, error)) {
        std::cerr << "first begin should succeed\n";
        return 12;
    }

    if (sink.begin(request, error) || error.empty()) {
        std::cerr << "second begin should fail\n";
        return 13;
    }

    sink.abort();

    if (!sink.begin(request, error) || !error.empty()) {
        std::cerr << "begin after abort should succeed\n";
        return 14;
    }

    sink.abort();
    return 0;
}

int test_reports_unknown_source_and_severity()
{
    std::ostringstream output;
    logscan::ConsoleReportSink sink(output);
    logscan::ScanRequest request;
    std::string error;

    if (!sink.begin(request, error)) {
        std::cerr << "sink failed to begin\n";
        return 20;
    }

    logscan::BatchReport batch;
    batch.findings.push_back(
        make_finding(
            1,
            "",
            0,
            logscan::Severity::Unknown,
            "unclassified"));
    if (!sink.consume(std::move(batch), error)) {
        std::cerr << "sink failed to consume unknown finding\n";
        return 21;
    }

    logscan::ScanReport report;
    if (!sink.end(report, error)) {
        std::cerr << "sink failed to end\n";
        return 22;
    }

    const std::string expected =
        "<unknown>:0 [UNKNOWN] unclassified\n"
        "summary: batches=0 records=0 analyzed=0 findings=0\n";
    if (output.str() != expected) {
        std::cerr << "unknown values were not rendered predictably\n";
        return 23;
    }

    return 0;
}

int test_rejects_unwritable_stream()
{
    std::ostringstream output;
    output.setstate(std::ios::badbit);

    logscan::ConsoleReportSink sink(output);
    logscan::ScanRequest request;
    std::string error;
    if (sink.begin(request, error) || error.empty()) {
        std::cerr << "unwritable stream should be rejected\n";
        return 30;
    }

    return 0;
}

}  // namespace

int main()
{
    if (const int result = test_writes_findings_and_summary(); result != 0) {
        return result;
    }
    if (const int result = test_rejects_invalid_lifecycle(); result != 0) {
        return result;
    }
    if (const int result = test_reports_unknown_source_and_severity();
        result != 0) {
        return result;
    }
    if (const int result = test_rejects_unwritable_stream(); result != 0) {
        return result;
    }
    return 0;
}
