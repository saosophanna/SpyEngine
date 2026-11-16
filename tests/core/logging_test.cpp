#include "core/logging/log_formatter.hpp"
#include "core/logging/log_macros.hpp"
#include "core/logging/logger.hpp"
#include "core/logging/sinks/file_sink.hpp"

#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <system_error>

namespace {

int g_failures = 0;

void check(bool condition, const char* expr, int line) {
    if (!condition) {
        std::fprintf(stderr, "FAILED: %s (line %d)\n", expr, line);
        ++g_failures;
    }
}

std::string readFile(const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
}

}

#define SPY_CHECK(expr) check((expr), #expr, __LINE__)

int main() {
    using namespace spy;

    std::error_code error;
    const std::filesystem::path syncPath =
        std::filesystem::temp_directory_path() / "spy_logging_sync_test.log";
    const std::filesystem::path asyncPath =
        std::filesystem::temp_directory_path() / "spy_logging_async_test.log";
    std::filesystem::remove(syncPath, error);
    std::filesystem::remove(asyncPath, error);

    LogManager& manager = LogManager::instance();
    manager.configure(LogConfig{LogLevel::Trace, false, 1024, false, true});
    manager.clearSinks();

    LogMessage message;
    message.level = LogLevel::Warn;
    message.context = LogContext::fromSource({"file.cpp", 7, "fn"}, "test",
                                             std::chrono::system_clock::now(), 1);
    message.text = "hello";

    LogFormatter formatter;
    const std::string formatted = formatter.format(message);
    SPY_CHECK(formatted.find("hello") != std::string::npos);
    SPY_CHECK(formatted.find("WRN") != std::string::npos);
    SPY_CHECK(formatted.find("test") != std::string::npos);
    SPY_CHECK(formatted.find("file.cpp:7") != std::string::npos);

    formatter.setPattern("{level}|{category}|{message}");
    SPY_CHECK(formatter.format(message) == "warn|test|hello");

    formatter.setPattern("{unknown}");
    SPY_CHECK(formatter.format(message) == "{unknown}");

    SPY_CHECK(logLevelEnabled(LogLevel::Error, LogLevel::Info));
    SPY_CHECK(!logLevelEnabled(LogLevel::Debug, LogLevel::Info));
    SPY_CHECK(!logLevelEnabled(LogLevel::Off, LogLevel::Trace));
    SPY_CHECK(!logLevelEnabled(LogLevel::Error, LogLevel::Off));
    SPY_CHECK(logLevelFromString("warn") == LogLevel::Warn);
    SPY_CHECK(!logLevelFromString("nope").has_value());

    auto syncSink = std::make_unique<FileSink>(syncPath, false);
    SPY_CHECK(syncSink->isOpen());
    manager.addSink(std::move(syncSink));

    Logger logger("suite");
    logger.setLevel(LogLevel::Info);
    SPY_CHECK(!logger.shouldLog(LogLevel::Debug));
    SPY_CHECK(logger.shouldLog(LogLevel::Info));

    logger.warn(SPY_LOG_SOURCE, formatLog("value {}", 42));
    SPY_LOG_INFO_TO(logger, "from macro {}", 1);
    manager.flush();

    const std::string syncContent = readFile(syncPath);
    SPY_CHECK(syncContent.find("value 42") != std::string::npos);
    SPY_CHECK(syncContent.find("from macro 1") != std::string::npos);
    SPY_CHECK(syncContent.find("[suite]") != std::string::npos);
    SPY_CHECK(syncContent.find("DBG") == std::string::npos);

    manager.clearSinks();
    auto asyncSink = std::make_unique<FileSink>(asyncPath, false);
    SPY_CHECK(asyncSink->isOpen());
    manager.addSink(std::move(asyncSink));
    manager.configure(LogConfig{LogLevel::Trace, true, 1024, false, true});

    SPY_LOG_ERROR_TO(logger, "async {}", 7);
    manager.flush();

    const std::string asyncContent = readFile(asyncPath);
    SPY_CHECK(asyncContent.find("async 7") != std::string::npos);

    manager.clearSinks();
    manager.configure(LogConfig{LogLevel::Info, false, 1024, false, true});
    SPY_LOG_INFO("after reconfigure");

    std::filesystem::remove(syncPath, error);
    std::filesystem::remove(asyncPath, error);

    if (g_failures == 0) {
        std::puts("all logging tests passed");
        return 0;
    }
    std::fprintf(stderr, "%d check(s) failed\n", g_failures);
    return 1;
}
