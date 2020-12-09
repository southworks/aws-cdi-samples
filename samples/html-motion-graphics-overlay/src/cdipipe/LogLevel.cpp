#include "LogLevel.h"

enum_map<LogLevel> log_level_map{
    { "None", LogLevel::None },
    { "Trace", LogLevel::Trace },
    { "Debug", LogLevel::Debug },
    { "Info", LogLevel::Info },
    { "Warning", LogLevel::Warning },
    { "Error", LogLevel::Error },
    { "Message", LogLevel::Message }
};
