#pragma once

#include "core/logging/log_sink.hpp"

#include <filesystem>
#include <fstream>
#include <mutex>
#include <string_view>

namespace spy {

class FileSink final : public LogSink {
public:
    explicit FileSink(const std::filesystem::path& path, bool append = true);
    ~FileSink() override;

    FileSink(const FileSink&) = delete;
    FileSink& operator=(const FileSink&) = delete;

    [[nodiscard]] bool isOpen() const noexcept { return m_stream.is_open(); }
    [[nodiscard]] const std::filesystem::path& path() const noexcept { return m_path; }

    void write(const LogMessage& message, std::string_view formatted) override;
    void flush() override;

private:
    std::filesystem::path m_path;
    std::ofstream m_stream;
    std::mutex m_mutex;
};

}
