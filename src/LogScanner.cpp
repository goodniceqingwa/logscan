#include "logscan/LogScanner.h"

#include <atomic>
#include <utility>

namespace logscan {

// 扫描器内部实现的接口占位。
// 这里保留生命周期、扫描和取消入口，具体并发编排由后续实现补齐。
class LogScanner::Impl {
public:
    Impl(LogScannerConfig config, ScannerComponents components);
    ~Impl();

    Impl(const Impl&) = delete;
    Impl& operator=(const Impl&) = delete;
    Impl(Impl&&) noexcept;
    Impl& operator=(Impl&&) noexcept;

    ScanResult scan(const ScanRequest& request);
    void cancel() noexcept;

private:
    LogScannerConfig config_;
    ScannerComponents components_;
    std::atomic_bool cancel_requested_{false};
};

LogScanner::Impl::Impl(LogScannerConfig config, ScannerComponents components)
    : config_(std::move(config)),
      components_(std::move(components))
{
}

LogScanner::Impl::~Impl() = default;

LogScanner::Impl::Impl(Impl&& other) noexcept
    : config_(std::move(other.config_)),
      components_(std::move(other.components_)),
      cancel_requested_(other.cancel_requested_.load(std::memory_order_relaxed))
{
}

LogScanner::Impl& LogScanner::Impl::operator=(Impl&& other) noexcept
{
    if (this != &other) {
        config_ = std::move(other.config_);
        components_ = std::move(other.components_);
        cancel_requested_.store(
            other.cancel_requested_.load(std::memory_order_relaxed),
            std::memory_order_relaxed);
    }
    return *this;
}

ScanResult LogScanner::Impl::scan(const ScanRequest& request)
{
    if (cancel_requested_.load(std::memory_order_relaxed)) {
        return ScanResult{
            ScanStatus::Cancelled,
            ScanReport{},
            "LogScanner: scan was cancelled by the caller."};
    }

    if (request.inputs.empty()) {
        return ScanResult{
            ScanStatus::Failed,
            ScanReport{},
            "LogScanner: no input paths were provided."};
    }

    std::string missing_components;
    const auto append_missing = [&missing_components](
        const char* name,
        bool missing) {
        if (!missing) {
            return;
        }
        if (!missing_components.empty()) {
            missing_components += ", ";
        }
        missing_components += name;
    };

    append_missing("LogSource", !components_.source);
    append_missing("LogParser", !components_.parser);
    append_missing("LogAnalyzer", !components_.analyzer);
    append_missing("LogReportSink", !components_.report_sink);

    if (!missing_components.empty()) {
        return ScanResult{
            ScanStatus::Failed,
            ScanReport{},
            "LogScanner: missing required component(s): " + missing_components + "."};
    }

    return ScanResult{
        ScanStatus::Failed,
        ScanReport{},
        "LogScanner: the scan pipeline is not implemented yet."};
}

void LogScanner::Impl::cancel() noexcept
{
    cancel_requested_.store(true, std::memory_order_relaxed);
}

LogScanner::LogScanner(LogScannerConfig config, ScannerComponents components)
    : impl_(std::make_unique<Impl>(std::move(config), std::move(components)))
{
}

LogScanner::~LogScanner() = default;

LogScanner::LogScanner(LogScanner&&) noexcept = default;

LogScanner& LogScanner::operator=(LogScanner&&) noexcept = default;

ScanResult LogScanner::scan(const ScanRequest& request)
{
    if (!impl_) {
        return ScanResult{
            ScanStatus::Failed,
            ScanReport{},
            "LogScanner: scanner is not initialized (it may have been moved from)."};
    }
    return impl_->scan(request);
}

void LogScanner::cancel() noexcept
{
    if (impl_) {
        impl_->cancel();
    }
}

}  // namespace logscan
