#include "logscan/LogAnalyzer.h"
#include "logscan/LogParser.h"
#include "logscan/LogReportSink.h"
#include "logscan/LogScanner.h"
#include "logscan/LogScannerConfig.h"
#include "logscan/LogSource.h"
#include "logscan/LogTypes.h"

#include <type_traits>

static_assert(std::is_abstract<logscan::LogSource>::value,
              "LogSource is an extension point");
static_assert(std::is_abstract<logscan::LogParser>::value,
              "LogParser is an extension point");
static_assert(std::is_abstract<logscan::LogAnalyzer>::value,
              "LogAnalyzer is an extension point");
static_assert(std::is_abstract<logscan::LogReportSink>::value,
              "LogReportSink is an extension point");

int main() {
    logscan::LogScannerConfig config;
    logscan::ScanRequest request;
    logscan::RawLogBatch raw_batch;
    logscan::LogBatch parsed_batch;
    logscan::BatchReport batch_report;

    (void)config;
    (void)request;
    (void)raw_batch;
    (void)parsed_batch;
    (void)batch_report;
    return 0;
}
