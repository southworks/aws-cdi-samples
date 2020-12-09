#pragma once

#include <cdi_log_api.h>

#include "Logger.h"

namespace CdiTools
{
    class CdiLogger : public CdiLogMethodData
    {
    public:
        CdiLogger();
        CdiLogger(const char* file_path);
        CdiLogger(Logger* logger);
        CdiLogger(Logger* logger, const char* source);

    private:
        static void log_message_callback(const CdiLogMessageCbData* cb_data_ptr);

        Logger* logger_;
        const char* source_;
    };
}