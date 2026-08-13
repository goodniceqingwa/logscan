#pragma once

#include "logscan/LogAnalyzer.h"
#include "logscan/LogParser.h"
#include "logscan/LogReportSink.h"
#include "logscan/LogScannerConfig.h"
#include "logscan/LogSource.h"
#include "logscan/LogTypes.h"

#include <memory>

namespace logscan {

// 扫描流水线依赖集合。
// 构造 LogScanner 后，以下组件的所有权转移给扫描器。
// 每个组件都是抽象接口，调用方可以注入自己的输入、解析、分析和输出策略。
struct ScannerComponents {
    std::unique_ptr<LogSource> source;          // 协调线程拥有的输入源。
    std::unique_ptr<LogParser> parser;          // 用于 clone() 的解析器原型。
    std::unique_ptr<LogAnalyzer> analyzer;      // 用于 clone() 的分析器原型。
    std::unique_ptr<LogReportSink> report_sink; // collector 侧的结果接收器。
};

class LogScanner {
public:
    LogScanner(LogScannerConfig config, ScannerComponents components);
    ~LogScanner();

    LogScanner(const LogScanner&) = delete;
    LogScanner& operator=(const LogScanner&) = delete;
    LogScanner(LogScanner&&) noexcept;
    LogScanner& operator=(LogScanner&&) noexcept;

    // 预期流程：读取原始批次、投递给 worker、解析和分析、汇聚并输出结果。
    // 当前仅保留接口，未提供任何业务实现。
    ScanResult scan(const ScanRequest& request);

    // 请求取消。具体实现决定 worker 观察到取消信号的时机。
    void cancel() noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;  // 隐藏线程池、队列等实现细节。
};

}  // namespace logscan
