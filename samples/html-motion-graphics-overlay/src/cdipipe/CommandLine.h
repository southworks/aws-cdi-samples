#pragma once

#include <string>
#include <map>
#include <sstream>

#include "Enum.h"

class OptionBase
{
public:
    OptionBase(const std::string& name, const std::string& help_text)
        : name_{ name }
        , help_text_{ help_text }
    {
    }

    const std::string& get_name() { return name_; }
    virtual std::string get_help_text() { return help_text_; }
    virtual bool set_value(const std::string& option_value) = 0;
    virtual bool is_switch() const = 0;

    friend std::ostream& operator<<(std::ostream& os, const OptionBase& option);

private:
    virtual void write(std::ostream& os) const = 0;

    std::string name_;
    std::string help_text_;
};

template <typename T>
class Option : public OptionBase
{
public:
    Option(const std::string& name, const std::string& help_text, T& value)
        : OptionBase(name, help_text)
        , value_{ value }
    {
    }

    bool set_value(const std::string& option_value) override;
    inline const T& get_value() const { return value_; }
    inline bool is_switch() const override { return std::is_integral<T>::value && std::is_same<T, bool>::value; }

private:
    void write(std::ostream& os) const override;

    T& value_;
};

template <typename T>
bool Option<T>::set_value(const std::string& option_value)
{
    std::istringstream(option_value) >> value_;

    return true;
}

template<>
bool Option<bool>::set_value(const std::string& option_value);

template <typename T>
void Option<T>::write(std::ostream& os) const
{
    os << value_;
}

template <typename T>
class EnumOption : public OptionBase
{
public:
    EnumOption(const std::string& name, const std::string& help_text, T& value, const enum_map<T>& value_map)
        : OptionBase(name, help_text)
        , value_{ value }
        , value_map_{ value_map }
    {
    }

    bool set_value(const std::string& option_value) override;
    inline bool is_switch() const override { return std::is_integral<T>::value && std::is_same<T, bool>::value; }
    std::string get_help_text() override;

private:
    void write(std::ostream& os) const override;

    T& value_;
    const enum_map<T>& value_map_;
};

template <typename T>
bool EnumOption<T>::set_value(const std::string& option_value)
{
    auto value = value_map_.find(option_value);
    if (value != value_map_.end()) {
        value_ = value->second;
        return true;
    }

    return false;
}

template <typename T>
std::string EnumOption<T>::get_help_text()
{
    std::ostringstream help_text;
    help_text << OptionBase::get_help_text();
    help_text << " [";
    bool is_list = false;
    for (auto& name : value_map_) {
        if (is_list) {
            help_text << ", ";
        }

        help_text << name.first;

        is_list = true;
    }

    help_text << "]";

    return help_text.str();
}

template <typename T>
void EnumOption<T>::write(std::ostream& os) const
{
    os << enum_name(value_map_, value_);
}

class CommandLine
{
public:
    //typedef std::function<bool(const OptionBase& option)> Validator;

    CommandLine(std::string program_description = "")
        : program_description_{ program_description }
    {
    }

    bool parse(int argc, char* argv[]);
    bool show_usage();
    void show_options();
    template <typename T> CommandLine& add_option(const std::string& name, const std::string& help_text, T& value);
    template <typename T> CommandLine& add_option(const std::string& name, const std::string& help_text, T& value, const enum_map<T>& value_map);
    //CommandLine& add_validator(const std::string& name, Validator validator);

private:
    std::string program_path_;
    std::string program_description_;
    std::map<std::string, std::unique_ptr<OptionBase>> options_;
    //std::multimap<std::string, Validator> validators_;
};

template <typename T>
CommandLine& CommandLine::add_option(const std::string& name, const std::string& help_text, T& value)
{
    options_[name] = std::make_unique<Option<T>>(name, help_text, value);
    //if (is_required) {
    //    validators_.insert(std::make_pair(name, [](const OptionBase& option) {
    //        return false;
    //    }));
    //}

    return *this;
}

template <typename T>
CommandLine& CommandLine::add_option(const std::string& name, const std::string& help_text, T& value, const enum_map<T>& value_map)
{
    options_[name] = std::make_unique<EnumOption<T>>(name, help_text, value, value_map);

    return *this;
}
