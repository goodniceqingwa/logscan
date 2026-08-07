#pragma once

#include "logscan/LogTypes.h"

#include <string>

namespace logscan {

enum class SourceReadStatus : std::uint8_t {
    BatchReady,
    EndOfInput,
    Error,
};

struct SourceReadResult {
    SourceReadStatus status{SourceReadStatus::EndOfInput};
    std::string error;
};

class LogSource {
public:
    virtual ~LogSource() = default;

    // Called by the coordinator before worker threads are started.
    virtual bool open(const ScanRequest& request, std::string& error) = 0;

    // The source is owned by the coordinator; it does not need to be
    // thread-safe. A successful BatchReady result must fill the output batch.
    virtual SourceReadResult read(RawLogBatch& batch) = 0;

    // Called after completion, failure, or cancellation.
    virtual void close() noexcept = 0;
};

}  // namespace logscan
