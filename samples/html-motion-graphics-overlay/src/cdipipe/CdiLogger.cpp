#include "CdiLogger.h"
#include "Cdi.h"

CdiTools::CdiLogger::CdiLogger()
    : logger_{ nullptr }
    , source_{ nullptr }
{
    log_method = kLogMethodStdout;
}

CdiTools::CdiLogger::CdiLogger(const char* file_path)
    : logger_{ nullptr }
    , source_{ nullptr }
{
    log_method = kLogMethodFile;
    log_filename_str = file_path;
}

CdiTools::CdiLogger::CdiLogger(Logger* logger)
    : logger_{ logger }
    , source_{ nullptr }
{
    log_method = kLogMethodCallback;
    callback_data.log_msg_cb_ptr = &log_message_callback;
    callback_data.log_user_cb_param = this;
}

CdiTools::CdiLogger::CdiLogger(Logger* logger, const char* source)
    : logger_{ logger }
    , source_{ source }
{
    log_method = kLogMethodCallback;
    callback_data.log_msg_cb_ptr = &log_message_callback;
    callback_data.log_user_cb_param = this;
}

void CdiTools::CdiLogger::log_message_callback(const CdiLogMessageCbData* cb_data_ptr)
{
    CdiLogger* self = ((CdiLogger*)cb_data_ptr->log_user_cb_param);
    self->logger_->log(Cdi::map_log_level(cb_data_ptr->log_level)) << cb_data_ptr->message_str;
}
