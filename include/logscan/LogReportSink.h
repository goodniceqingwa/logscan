#pragma once

#include "logscan/LogTypes.h"

#include <string>

namespace logscan {

class LogReportSink {
public:
    virtual ~LogReportSink() = default;

    // Sink callbacks are intended to run on the collector side. This keeps
    // output ordering and external I/O out of worker threads by default.
    virtual bool begin(const ScanRequest& request, std::string& error) = 0;
    virtual bool consume(BatchReport&& report, std::string& error) = 0;
    virtual bool end(const ScanReport& report, std::string& error) = 0;
    virtual void abort() noexcept = 0;
};

}  // namespace logscan
