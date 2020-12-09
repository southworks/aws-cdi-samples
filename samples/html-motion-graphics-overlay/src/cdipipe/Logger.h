#pragma once

#include <sstream>
#include <fstream>

#include <boost/asio/io_context.hpp>

#include "LogLevel.h"

class Logger;

class LogEntry : public std::stringstream
{
public:
    LogEntry(LogLevel log_level, Logger& logger, const std::string& source = "")
        : logger_{ logger }
        , log_level_{ log_level }
        , source_{ source } { }

    LogEntry(LogEntry&& log) noexcept
        : log_level_{ log.log_level_ }
        , logger_{ log.logger_ }
        , source_{ log.source_ } { }

    ~LogEntry();

private:
    Logger& logger_;
    LogLevel log_level_;
    const std::string source_;
};

class Logger
{
public:
    Logger(const std::string& source);

    static void start(LogLevel log_level = LogLevel::Info, const std::string& file_name = "", bool show_timestamp = true, bool show_thread_id = false);
    static void shutdown();
    static inline void set_level(LogLevel log_level) { log_level_ = log_level; }

    inline LogEntry trace() { return LogEntry{ LogLevel::Trace, *this, source_ }; }
    inline LogEntry debug() { return LogEntry{ LogLevel::Debug, *this, source_ }; }
    inline LogEntry info() { return LogEntry{ LogLevel::Info, *this, source_ }; }
    inline LogEntry warning() { return LogEntry{ LogLevel::Warning, *this, source_ }; }
    inline LogEntry error() { return LogEntry{ LogLevel::Error, *this, source_ }; }

    inline LogEntry write() { return LogEntry{ LogLevel::Message, *this, source_ }; }
    inline LogEntry log(LogLevel log_level) { return LogEntry{ log_level, *this, source_ }; }
    inline LogEntry log(const std::string& source, LogLevel log_level) { return LogEntry{ log_level, *this, source }; }

private:
    friend class LogEntry;

    static LogLevel log_level_;
    static std::thread logger_thread_;
    static boost::asio::io_context io_;
    static boost::asio::io_context::strand strand_;
    static std::unique_ptr<boost::asio::io_context::work> active_;
    static bool show_timestamp_;
    static const char* timestamp_format_;
    static bool show_thread_id_;
    static bool uses_console_;
    static std::filebuf log_file_;
    static std::ostream stream_;

    LogEntry create_log_entry(LogLevel log_level, const std::string& source = "");
    const char* get_severity(LogLevel log_level);
    int get_color(LogLevel log_level);
    void write(LogLevel log_level, const std::string& source, std::stringstream& log_entry_text);
    void flush(LogLevel log_level, const std::string& source, const std::string& message, std::thread::id thread_id);

    const std::string source_;
};

#define LOG_WRITE           logger_.write()
#define LOG_TRACE           logger_.trace()
#define LOG_DEBUG           logger_.debug()
#define LOG_INFO            logger_.info()
#define LOG_STATUS          logger_.info()
#define LOG_WARNING         logger_.warning()
#define LOG_ERROR           logger_.error()
#define LOG(level, source)  logger_.log(source, level)
