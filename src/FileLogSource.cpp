#include "logscan/FileLogSource.h"

namespace logscan{

FileLogSource::FileLogSource(std::size_t batch_size)
    : batch_size_(batch_size)
{
    if (batch_size_ == 0) {
        throw std::invalid_argument(
            "FileLogSource: batch_size must be greater than zero.");
    }
}

bool FileLogSource::open(const ScanRequest& request, std::string& error)
{
    close();
    error.clear();

    //0.1.0
    if (request.inputs.size() != 1)
    {
        error = "FileLogSource : exactly one input file is required";
        return false;
    }

    const auto& path  = request.inputs.front();

    //判断时候为正常文件  错误值保存在filesystem_error
    std::error_code filesystem_error;
    const bool is_regular_file  = std::filesystem::is_regular_file(path, filesystem_error);

    if (filesystem_error)
    {
        error = "FileLogSourece: cannot inspect input file '" + path.string() + "' :" + filesystem_error.message();
        return false;
    }

    if (!is_regular_file)
    {
        error = "FileLogSource: input is not a regular file '" + path.string() + "'.";
        return false;
    }

    stream_.open(path, std::ios::in | std::ios::binary);

    if (!stream_.is_open())
    {
        error = "FileLogSource: cannot open input file '" + path.string() + "'.";
        return false;
    }

    current_path_ = path;
    next_batch_id_ = 0;
    next_record_id_ = 0;
    next_line_number_ = 1;

    return true;
}

SourceReadResult FileLogSource::read(RawLogBatch& batch)
{
    batch = RawLogBatch{};

    if (!stream_.is_open())
    {
        return SourceReadResult{SourceReadStatus::Error, "FileLogSource: no input file is open."};
    }

    RawLogBatch next_batch;
    next_batch.id = next_batch_id_;
    next_batch.lines.reserve(batch_size_);

    while (next_batch.lines.size() < batch_size_)
    {
        // 对应字节的指针
        const std::streampos line_offset = stream_.tellg();

        std::string text;

        if (!std::getline(stream_, text))
        {
            if (stream_.eof())
            {
                break;
            }

            return SourceReadResult{SourceReadStatus::Error, "FileLogSource: failed while reading '" + current_path_.string() + "'."};
        }

        if (!text.empty() && text.back() == '\r')
        {
            text.pop_back();
        }

        RawLogLine line;
        line.id = next_record_id_;
        line.position.source = current_path_;
        line.position.line = next_line_number_;

        if (line_offset != std::streampos{-1})
        {
            line.position.byte_offset = static_cast<std::uint64_t>(static_cast<std::streamoff>(line_offset));
        }

        line.text = std::move(text);

        next_batch.lines.push_back(std::move(line));

        ++next_record_id_;
        ++next_line_number_;
    }

    if (!next_batch.lines.empty())
    {
        ++next_batch_id_;
        batch = std::move(next_batch);

        return SourceReadResult{SourceReadStatus::BatchReady, {}};
    }

    if (stream_.eof())
    {
        return SourceReadResult{SourceReadStatus::EndOfInput, {}};
    }

    return SourceReadResult{SourceReadStatus::Error, "FileLogSource: input stream entered an invalid state"};

}

void FileLogSource::close() noexcept
{
    if(stream_.is_open())
    {
        stream_.close();
    }

    current_path_.clear();
    next_batch_id_ = 0;
    next_record_id_ = 0;
    next_line_number_ = 1;
}


}  // namespace logscan
