#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace logscan {

using BatchId = std::uint64_t;
using RecordId = std::uint64_t;

enum class Severity : std::uint8_t {
    Unknown,
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Critical,
};

struct SourcePosition {
    std::filesystem::path source;
    std::uint64_t line{0};
    std::uint64_t byte_offset{0};
};

struct RawLogLine {
    RecordId id{0};
    SourcePosition position;
    std::string text;
};

struct RawLogBatch {
    BatchId id{0};
    std::vector<RawLogLine> lines;
};

struct LogRecord {
    RecordId id{0};
    SourcePosition position;
    std::chrono::system_clock::time_point timestamp{};
    Severity severity{Severity::Unknown};
    std::string logger;
    std::string message;
    std::string raw_text;
};

struct LogBatch {
    BatchId id{0};
    std::size_t worker_id{0};
    std::vector<LogRecord> records;
};

struct Finding {
    RecordId record_id{0};
    SourcePosition position;
    Severity severity{Severity::Unknown};
    std::string rule_id;
    std::string message;
};

struct ScanStatistics {
    std::uint64_t batches_read{0};
    std::uint64_t records_read{0};
    std::uint64_t records_analyzed{0};
    std::uint64_t findings_emitted{0};
};

struct BatchReport {
    BatchId batch_id{0};
    std::size_t worker_id{0};
    std::uint64_t records_analyzed{0};
    std::vector<Finding> findings;
};

struct ScanReport {
    ScanStatistics statistics;
    std::vector<Finding> findings;
};

enum class ScanStatus : std::uint8_t {
    Completed,
    Failed,
    Cancelled,
};

struct ScanResult {
    ScanStatus status{ScanStatus::Failed};
    ScanReport report;
    std::string error;
};

struct ScanRequest {
    std::vector<std::filesystem::path> inputs;
};

}  // namespace logscan
