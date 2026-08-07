#pragma once

#include "logscan/LogTypes.h"

#include <memory>
#include <string>

namespace logscan {

class LogAnalyzer {
public:
    virtual ~LogAnalyzer() = default;

    // Each worker receives its own analyzer instance. Per-worker state can be
    // accumulated locally and represented in the returned BatchReport.
    virtual std::unique_ptr<LogAnalyzer> clone() const = 0;

    // Analyze a parsed batch. The analyzer must not mutate shared state unless
    // it provides its own synchronization.
    virtual bool analyze(const LogBatch& input,
                         BatchReport& output,
                         std::string& error) = 0;
};

}  // namespace logscan
