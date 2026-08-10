#pragma once

#include "logscan/LogTypes.h"

#include <string>

namespace logscan {

// 批次结果的输出/持久化接口。
// 默认约定：所有回调由 collector 线程调用，避免 worker 直接执行外部 I/O。
class LogReportSink {
public:
    virtual ~LogReportSink() = default;

    // 扫描开始时调用。失败时应写入 error 并阻止后续处理。
    virtual bool begin(const ScanRequest& request, std::string& error) = 0;

    // 接收一个 worker 完成的局部报告。报告的所有权通过右值转移给 sink。
    virtual bool consume(BatchReport&& report, std::string& error) = 0;

    // 所有批次汇聚后调用，可用于刷新文件、提交事务或输出最终报告。
    virtual bool end(const ScanReport& report, std::string& error) = 0;

    // 发生错误或取消时调用，用于清理未完成的输出状态。
    virtual void abort() noexcept = 0;
};

}  // namespace logscan
