#pragma once

#include "logscan/LogAnalyzer.h"
#include "logscan/LogParser.h"
#include "logscan/LogReportSink.h"
#include "logscan/LogScannerConfig.h"
#include "logscan/LogSource.h"
#include "logscan/LogTypes.h"

#include <memory>

namespace logscan {

// Components are owned by LogScanner after construction.
// Each component is intentionally abstract so callers can provide their own
// input, parsing, analysis, and output policies.
struct ScannerComponents {
    std::unique_ptr<LogSource> source;
    std::unique_ptr<LogParser> parser;
    std::unique_ptr<LogAnalyzer> analyzer;
    std::unique_ptr<LogReportSink> report_sink;
};

class LogScanner {
public:
    LogScanner(LogScannerConfig config, ScannerComponents components);
    ~LogScanner();

    LogScanner(const LogScanner&) = delete;
    LogScanner& operator=(const LogScanner&) = delete;
    LogScanner(LogScanner&&) noexcept;
    LogScanner& operator=(LogScanner&&) noexcept;

    // The implementation is expected to read, partition, process, and merge
    // batches according to LogScannerConfig.
    ScanResult scan(const ScanRequest& request);

    // Cancellation is part of the orchestration contract. The concrete
    // implementation decides how quickly workers observe the request.
    void cancel() noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace logscan
