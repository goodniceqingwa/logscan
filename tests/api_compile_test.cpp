#include "logscan/LogAnalyzer.h"
#include "logscan/LogParser.h"
#include "logscan/LogReportSink.h"
#include "logscan/LogScanner.h"
#include "logscan/LogScannerConfig.h"
#include "logscan/LogSource.h"
#include "logscan/LogTypes.h"

#include <type_traits>
#include <utility>
#include <cassert>

// 这些断言确保四个扩展点保持抽象接口，避免框架意外提供默认业务实现。
static_assert(std::is_abstract<logscan::LogSource>::value,
              "LogSource is an extension point");
static_assert(std::is_abstract<logscan::LogParser>::value,
              "LogParser is an extension point");
static_assert(std::is_abstract<logscan::LogAnalyzer>::value,
              "LogAnalyzer is an extension point");
static_assert(std::is_abstract<logscan::LogReportSink>::value,
              "LogReportSink is an extension point");

int main() {
    // 这里只验证公共数据契约可以被客户端包含和构造，不执行任何扫描行为。
    logscan::LogScannerConfig config;
    logscan::ScannerComponents components;
    logscan::LogScanner scanner(std::move(config), std::move(components));
    logscan::ScanRequest request;
    logscan::RawLogBatch raw_batch;
    logscan::LogBatch parsed_batch;
    logscan::BatchReport batch_report;

    const auto result = scanner.scan(request);

    assert(result.status == logscan::ScanStatus::Failed);
    assert(!result.error.empty());
    scanner.cancel();

    (void)config;
    (void)request;
    (void)raw_batch;
    (void)parsed_batch;
    (void)batch_report;
    return 0;
}
