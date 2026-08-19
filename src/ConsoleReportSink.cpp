#include "logscan/ConsoleReportSink.h"

#include <ostream>
#include <utility>

namespace logscan {

ConsoleReportSink::ConsoleReportSink(std::ostream& output) noexcept
    : output_(output)
{
}

bool ConsoleReportSink::begin(
    const ScanRequest& request,
    std::string& error)
{
    (void)request;
    error.clear();

    if (active_) {
        error = "report sink is already active";
        return false;
    }

    if (!output_) {
        error = "output stream is not writable";
        return false;
    }

    active_ = true;
    return true;
}

bool ConsoleReportSink::consume(
    BatchReport&& report,
    std::string& error)
{
    error.clear();

    if (!active_) {
        error = "report sink is not active";
        return false;
    }

    for (const auto& finding : report.findings) {
        if (!write_finding(finding)) {
            error = "failed to write finding";
            return false;
        }
    }

    return true;
}

bool ConsoleReportSink::end(
    const ScanReport& report,
    std::string& error)
{
    error.clear();

    if (!active_) {
        error = "report sink is not active";
        return false;
    }

    const auto& statistics = report.statistics;
    output_
        << "summary: batches=" << statistics.batches_read
        << " records=" << statistics.records_read
        << " analyzed=" << statistics.records_analyzed
        << " findings=" << statistics.findings_emitted
        << '\n';
    output_.flush();

    if (!output_) {
        error = "failed to write scan summary";
        return false;
    }

    active_ = false;
    return true;
}

void ConsoleReportSink::abort() noexcept
{
    active_ = false;
}

const char* ConsoleReportSink::severity_name(Severity severity) noexcept
{
    switch (severity) {
    case Severity::Trace:
        return "TRACE";
    case Severity::Debug:
        return "DEBUG";
    case Severity::Info:
        return "INFO";
    case Severity::Warning:
        return "WARNING";
    case Severity::Error:
        return "ERROR";
    case Severity::Critical:
        return "CRITICAL";
    case Severity::Unknown:
        return "UNKNOWN";
    }

    return "UNKNOWN";
}

bool ConsoleReportSink::write_finding(const Finding& finding)
{
    if (finding.position.source.empty()) {
        output_ << "<unknown>";
    } else {
        output_ << finding.position.source.string();
    }

    output_
        << ':' << finding.position.line
        << " [" << severity_name(finding.severity) << "] "
        << finding.message
        << '\n';

    return static_cast<bool>(output_);
}

}  // namespace logscan
