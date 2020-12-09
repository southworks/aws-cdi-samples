#pragma once

#include <string>
#include <iostream>
#include <system_error>

namespace CdiTools
{
    enum class connection_error
    {
        not_connected = 100,
        already_connected,
        connection_failure,
        receive_error,
        transmit_error,
        no_buffer_space,
        bad_stream_identifier,
        bad_configuration
    };

    class connection_category
        : public std::error_category
    {
    public:
        virtual const char* name() const noexcept override final;
        virtual std::string message(int ev) const override final;
        virtual std::error_condition default_error_condition(int ev) const noexcept override final;
    };

    std::error_code make_error_code(CdiTools::connection_error ec);
    std::error_condition make_error_condition(CdiTools::connection_error ec);
}

namespace std
{
    template <>
    struct is_error_code_enum<CdiTools::connection_error> : public true_type {};
}
