#pragma once

#include "logscan/LogTypes.h"

#include <memory>
#include <string>

namespace logscan {

// 对结构化日志批次执行规则匹配、统计或其他分析的接口。
class LogAnalyzer {
public:
    virtual ~LogAnalyzer() = default;

    // 创建一个 worker 私有的分析器副本。
    // worker 内的局部统计应通过 BatchReport 或后续汇聚阶段合并。
    virtual std::unique_ptr<LogAnalyzer> clone() const = 0;

    // 分析一个已解析批次并填充局部结果。
    // 除非自行同步，否则实现不应修改跨 worker 共享状态。
    virtual bool analyze(const LogBatch& input,
                         BatchReport& output,
                         std::string& error) = 0;
};

}  // namespace logscan
