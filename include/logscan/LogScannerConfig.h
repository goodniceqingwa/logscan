#pragma once

#include <cstddef>

namespace logscan {

struct ParallelConfig {
    // 设为 0 时由实现根据平台或调用方策略决定 worker 数量。
    std::size_t worker_count{0};

    // 设为 0 时由实现决定每个 RawLogBatch 的大小。
    std::size_t batch_size{0};

    // 设为 0 时由实现决定队列中允许同时存在的批次数量。
    std::size_t max_in_flight_batches{0};

    // 为 true 时，收集端应依据 BatchId 恢复结果发布顺序。
    bool preserve_batch_order{true};
};

// 扫描器级别配置。后续可在这里加入输入、错误策略等非组件配置。
struct LogScannerConfig {
    ParallelConfig parallel{};
};

}  // namespace logscan
