#pragma once

#include "logscan/LogTypes.h"

#include <memory>
#include <string>

namespace logscan {

class LogParser {
public:
    virtual ~LogParser() = default;

    // The returned parser belongs to one worker and must not be shared with
    // another worker unless the implementation explicitly guarantees safety.
    virtual std::unique_ptr<LogParser> clone() const = 0;

    // Convert one raw batch into records. The caller assigns worker_id before
    // handing the parsed batch to the analyzer.
    virtual bool parse(const RawLogBatch& input,
                       LogBatch& output,
                       std::string& error) = 0;
};

}  // namespace logscan
