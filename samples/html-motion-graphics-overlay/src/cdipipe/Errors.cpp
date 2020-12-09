#include "Errors.h"

const char* CdiTools::connection_category::name() const noexcept
{
    return "connection";
}

std::string CdiTools::connection_category::message(int ev) const
{
    switch (static_cast<connection_error>(ev))
    {
    case connection_error::not_connected:
        return "connection is not ready";
    case connection_error::already_connected:
        return "connection is already established";
    case connection_error::connection_failure:
        return "connection has failed";
    case connection_error::receive_error:
        return "error receiving a payload";
    case connection_error::transmit_error:
        return "error sending a payload";
    case connection_error::no_buffer_space:
        return "payload buffer could not be allocated";
    case connection_error::bad_stream_identifier:
        return "stream identifier is invalid";
    case connection_error::bad_configuration:
        return "configuration is invalid";
    default:
        return "unknown connection error";
    }
}

// allow comparison with generic error conditions
std::error_condition CdiTools::connection_category::default_error_condition(int ev) const noexcept
{
    switch (static_cast<connection_error>(ev))
    {
    case connection_error::not_connected:
        return make_error_condition(std::errc::not_connected);
    case connection_error::already_connected:
        return make_error_condition(std::errc::already_connected);
    case connection_error::receive_error:
    case connection_error::transmit_error:
        return make_error_condition(std::errc::io_error);
    case connection_error::no_buffer_space:
        return make_error_condition(std::errc::no_buffer_space);
    default:
        return std::error_condition(ev, *this);
    }
}

const std::error_category& connection_category()
{
    static CdiTools::connection_category instance;

    return instance;
}

std::error_code CdiTools::make_error_code(CdiTools::connection_error ec)
{
    return std::error_code(static_cast<int>(ec), connection_category());
}

std::error_condition CdiTools::make_error_condition(CdiTools::connection_error ec)
{
    return std::error_condition(static_cast<int>(ec), connection_category());
}

