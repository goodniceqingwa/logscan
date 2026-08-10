#pragma once

#include "logscan/LogTypes.h"

#include <string>

namespace logscan {

// LogSource 的一次读取结果。BatchReady、EndOfInput 和 Error 必须由实现明确区分。
enum class SourceReadStatus : std::uint8_t {
    BatchReady,
    EndOfInput,
    Error,
};

// read() 的状态和失败原因。
struct SourceReadResult {
    SourceReadStatus status{SourceReadStatus::EndOfInput};
    std::string error;
};

// 输入适配器接口。
// 约定：open/read/close 只由协调线程调用，具体实现无需承担跨线程同步。
class LogSource {
public:
    virtual ~LogSource() = default;

    // 在 worker 启动前由协调线程调用，负责打开并校验 request 中的输入。
    virtual bool open(const ScanRequest& request, std::string& error) = 0;

    // 读取下一个原始批次。返回 BatchReady 时必须填充 batch。
    virtual SourceReadResult read(RawLogBatch& batch) = 0;

    // 扫描完成、失败或取消后调用，用于释放输入资源。
    virtual void close() noexcept = 0;
};

}  // namespace logscan
