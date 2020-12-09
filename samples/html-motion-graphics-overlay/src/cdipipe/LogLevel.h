#pragma once

#include "Enum.h"

enum class LogLevel {
    None,
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Message
};

extern enum_map<LogLevel> log_level_map;
