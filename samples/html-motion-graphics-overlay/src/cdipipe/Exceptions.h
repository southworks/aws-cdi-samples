#pragma once

#include <string>
#include <stdexcept>

namespace CdiTools
{
    class InvalidConfigurationException : public std::runtime_error {
    public:
        InvalidConfigurationException(const char* _Message) : std::runtime_error(_Message) { }
        InvalidConfigurationException(const std::string& _Message) : std::runtime_error(_Message) { }
    };

    class CdiInitializationException : public std::runtime_error {
    public:
        CdiInitializationException(const char* _Message) : std::runtime_error(_Message) { }
        CdiInitializationException(const std::string& _Message) : std::runtime_error(_Message) { }
    };
}