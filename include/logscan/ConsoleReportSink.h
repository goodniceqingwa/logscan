#pragma once

#include "logscan/LogReportSink.h"

#include <iosfwd>
#include <string>

namespace logscan {

// 将分析结果以稳定、面向人的文本格式写入输出流。
// 调用方必须保证传入的输出流比 sink 存活更久。
class ConsoleReportSink final : public LogReportSink {
public:
    explicit ConsoleReportSink(std::ostream& output) noexcept;

    bool begin(const ScanRequest& request, std::string& error) override;
    bool consume(BatchReport&& report, std::string& error) override;
    bool end(const ScanReport& report, std::string& error) override;
    void abort() noexcept override;

private:
    static const char* severity_name(Severity severity) noexcept;
    bool write_finding(const Finding& finding);

    std::ostream& output_;
    bool active_{false};
};

}  // namespace logscan
