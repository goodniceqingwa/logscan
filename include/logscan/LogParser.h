#pragma once

#include "logscan/LogTypes.h"

#include <memory>
#include <string>

namespace logscan {

// 原始文本到结构化 LogRecord 的解析器接口。
class LogParser {
public:
    virtual ~LogParser() = default;

    // 创建一个 worker 私有的解析器副本。
    // 默认约定是不跨 worker 共享可变解析状态。
    virtual std::unique_ptr<LogParser> clone() const = 0;

    // 将一个原始批次转换为结构化记录。
    // 调用方在交给分析器前负责设置 output.worker_id。
    virtual bool parse(const RawLogBatch& input,
                       LogBatch& output,
                       std::string& error) = 0;
};

}  // namespace logscan
