#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace logscan {

// 批次和记录的稳定标识。它们用于结果关联和可选的批次保序。
using BatchId = std::uint64_t;
using RecordId = std::uint64_t;

// 解析器识别出的日志级别。Unknown 用于无法识别或尚未解析的记录。
enum class Severity : std::uint8_t {
    Unknown,
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Critical,
};

// 原始日志在输入源中的位置。
struct SourcePosition {
    std::filesystem::path source;  // 文件名、设备名或其他输入源标识。
    std::uint64_t line{0};         // 行号；具体是否从 0 或 1 开始由输入源约定。
    std::uint64_t byte_offset{0};  // 在输入源中的字节偏移。
};

// 输入源交给解析器的最小数据单元，还没有结构化字段。
struct RawLogLine {
    RecordId id{0};  // 在本次扫描中的稳定记录编号。
    SourcePosition position;
    std::string text;
};

// 输入源按批次读取原始日志，批次是并行流水线的基本调度单元。
struct RawLogBatch {
    BatchId id{0};  // 用于结果排序和错误定位。
    std::vector<RawLogLine> lines;
};

// 解析器输出的结构化日志记录。
struct LogRecord {
    RecordId id{0};
    SourcePosition position;
    std::chrono::system_clock::time_point timestamp{};  // 无法解析时保持默认值。
    Severity severity{Severity::Unknown};
    std::string logger;    // 日志记录器、模块或组件名称。
    std::string message;   // 规范化后的日志正文。
    std::string raw_text;  // 可选：保留原始文本，便于诊断和回溯。
};

struct ParseIssue {
    RecordId record_id{0};
    SourcePosition position;
    std::string raw_text;
    std::string error;
};

// 解析后的批次。worker_id 用于定位产生结果的工作线程，不代表线程 ID。
struct LogBatch {
    BatchId id{0};
    std::size_t worker_id{0};
    std::vector<LogRecord> records;
    std::vector<ParseIssue> parse_issues;
};

// 分析规则对单条日志产生的结果。
struct Finding {
    RecordId record_id{0};
    SourcePosition position;
    Severity severity{Severity::Unknown};
    std::string rule_id;
    std::string message;
};

// 一次扫描的聚合统计。具体字段的累加策略由扫描器实现。
struct ScanStatistics {
    std::uint64_t batches_read{0};
    std::uint64_t records_read{0};
    std::uint64_t records_analyzed{0};
    std::uint64_t findings_emitted{0};
};

// 单个 worker 对一个批次产生的局部结果。
struct BatchReport {
    BatchId batch_id{0};
    std::size_t worker_id{0};
    std::uint64_t records_analyzed{0};
    std::vector<Finding> findings;
};

// 扫描完成后供调用方或输出端使用的整体报告。
struct ScanReport {
    ScanStatistics statistics;
    std::vector<Finding> findings;
};

// 扫描生命周期的最终状态。
enum class ScanStatus : std::uint8_t {
    Completed,
    Failed,
    Cancelled,
};

// 扫描入口的返回值。Failed 时应通过 error 提供可读错误信息。
struct ScanResult {
    ScanStatus status{ScanStatus::Failed};
    ScanReport report;
    std::string error;
};

// 一次扫描请求，可以包含一个或多个输入源。
struct ScanRequest {
    std::vector<std::filesystem::path> inputs;
};


}  // namespace logscan
