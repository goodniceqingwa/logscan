#ifndef FILELOGSOURCE_H
#define FILELOGSOURCE_H
#include "logscan/LogSource.h"

#include <cstddef>
#include <filesystem>
#include <fstream>

namespace logscan {

class FileLogSource final : public LogSource {
public:
    explicit FileLogSource(std::size_t batch_size = 1024);

    bool open(const ScanRequest& request, std::string& error) override;

    SourceReadResult read(RawLogBatch& batch) override;

    void close() noexcept override;

private:
    std::size_t batch_size_;                // 单次内存占用
    std::ifstream stream_;                  // 当前打开的输入流
    std::filesystem::path current_path_;    // 每条记录标记来源

    BatchId next_batch_id_{0};              // 后续处理时候排序
    RecordId next_record_id_{0};            // 解析结果关联原始数据
    std::uint64_t next_line_number_{1};     // 错误位置
};

}

#endif // FILELOGSOURCE_H
