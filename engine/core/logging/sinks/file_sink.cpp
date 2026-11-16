#include "core/logging/sinks/file_sink.hpp"

#include <system_error>

namespace spy {

FileSink::FileSink(const std::filesystem::path& path, bool append)
    : m_path(path) {
    std::error_code error;
    if (m_path.has_parent_path()) {
        std::filesystem::create_directories(m_path.parent_path(), error);
    }

    const std::ios::openmode mode = std::ios::out | (append ? std::ios::app : std::ios::trunc);
    m_stream.open(m_path, mode);
}

FileSink::~FileSink() {
    flush();
}

void FileSink::write(const LogMessage&, std::string_view formatted) {
    std::lock_guard lock(m_mutex);
    if (!m_stream.is_open()) {
        return;
    }
    m_stream << formatted << '\n';
}

void FileSink::flush() {
    std::lock_guard lock(m_mutex);
    if (m_stream.is_open()) {
        m_stream.flush();
    }
}

}
