#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <iomanip>
#include <boost/asio.hpp>

#ifdef _WIN32_WINNT
#include <Windows.h>
#endif

#include "Logger.h"

LogLevel Logger::log_level_{ LogLevel::Info };
std::thread Logger::logger_thread_;
boost::asio::io_context Logger::io_{ };
boost::asio::io_context::strand Logger::strand_{ io_ };
std::unique_ptr<boost::asio::io_context::work> Logger::active_;
bool Logger::show_timestamp_{ false };
const char* Logger::timestamp_format_{ "%T" };
bool Logger::show_thread_id_{ false };
bool Logger::uses_console_{ true };
std::filebuf Logger::log_file_{ };
std::ostream Logger::stream_{ NULL };

LogEntry::~LogEntry()
{
    logger_.write(log_level_, source_, *this);
}

LogEntry Logger::create_log_entry(LogLevel log_level, const std::string& source)
{
    return LogEntry(log_level, *this, source);
}

const char* Logger::get_severity(LogLevel log_level)
{
    switch (log_level)
    {
    case LogLevel::Trace:   return "TRACE";
    case LogLevel::Debug:   return "DEBUG";
    case LogLevel::Info:    return "INFO";
    case LogLevel::Warning: return "WARNING";
    case LogLevel::Error:   return "ERROR";
    case LogLevel::Message: return "";
    default:                return "UNKNOWN";
    }
}

int Logger::get_color(LogLevel log_level)
{
    switch (log_level) {
    case LogLevel::Trace: return 36;
    case LogLevel::Debug: return 32;
    case LogLevel::Info: return 37;
    case LogLevel::Warning: return 33;
    case LogLevel::Error: return 31;
    default: return 37;
    }
}

Logger::Logger(const std::string& source)
    : source_{ source }
{
}

void Logger::start(LogLevel log_level, const std::string& file_name, bool show_timestamp, bool show_thread_id)
{
#if defined WIN32 || defined _WIN32 || defined WIN64 || defined _WIN64
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
#endif

    log_level_ = log_level;
    uses_console_ = file_name.empty();
    show_timestamp_ = show_timestamp;
    show_thread_id = show_thread_id;
    if (uses_console_) {
        stream_.rdbuf(std::cerr.rdbuf());
    }
    else {
        log_file_.open(file_name, std::ios::out);
        stream_.rdbuf(&log_file_);
    }

    logger_thread_ = std::thread([&]() {
        active_ = std::make_unique<boost::asio::io_context::work>(io_);
        io_.run();
    });
}

void Logger::shutdown()
{
    active_.reset();
    if (logger_thread_.joinable()) {
        logger_thread_.join();
    }

    if (log_file_.is_open()) {
        log_file_.close();
    }
    
    std::cerr << "\033[0m";
}

void Logger::write(LogLevel log_level, const std::string& source, std::stringstream& log_entry)
{
    if (log_level_ > log_level) return;

    std::string message;
    std::stringstream log_entry_text;
    bool indent = false;
    while (std::getline(log_entry, message)) {
        if (indent) {
            log_entry_text << "\n" << std::setw(25) << std::setfill(' ') << "";
        }

        log_entry_text << message;
        indent = true;
    }

    strand_.post(std::bind(&Logger::flush, this, log_level, source, log_entry_text.str(), std::this_thread::get_id()));
}

void Logger::flush(LogLevel log_level, const std::string& source, const std::string& message, std::thread::id thread_id)
{
    if (uses_console_) {
        stream_ << "\033[" << get_color(log_level) << "m";
    }

    if (!source_.empty()) {
        stream_ << "[" << std::setw(12) << std::left << source << "] ";
    }

    if (show_timestamp_) {
        using namespace std::chrono;
        auto time_point = system_clock::now();
        auto time_t = system_clock::to_time_t(time_point);
        stream_ << std::put_time(std::localtime(&time_t), timestamp_format_)
            << "." << std::setw(6) << std::setfill('0') << std::right
            << duration_cast<microseconds>(time_point - system_clock::from_time_t(time_t)).count()
            << std::setfill(' ') << " ";
    }

    if (log_level != LogLevel::Message) {
        stream_ << std::setw(7) << std::left << get_severity(log_level) << ": ";
    }

    if (show_thread_id_) {
        stream_ << std::setw(5) << std::setfill('0') << std::right << thread_id << std::setfill(' ') << ": " ;
    }

    stream_ << message;

    if (message[0] != '\r') {
        stream_ << "\n";
    }

    if (uses_console_) {
        stream_ << "\033[0m";
    }

    stream_.flush();
}
