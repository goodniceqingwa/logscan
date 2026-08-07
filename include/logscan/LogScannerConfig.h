#pragma once

#include <cstddef>

namespace logscan {

struct ParallelConfig {
    // Zero means that the implementation chooses a platform-appropriate
    // value. The framework does not prescribe a thread-count policy.
    std::size_t worker_count{0};

    // Zero means that the implementation chooses a batch size.
    std::size_t batch_size{0};

    // Zero means that the implementation chooses the in-flight limit.
    std::size_t max_in_flight_batches{0};

    // If true, the collector must use BatchId when publishing results.
    bool preserve_batch_order{true};
};

struct LogScannerConfig {
    ParallelConfig parallel{};
};

}  // namespace logscan
