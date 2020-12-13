#include <string>
#include <memory>
#include <iostream>
#include <iomanip>

#include "CommandLine.h"

std::ostream& operator<<(std::ostream& os, const OptionBase& option)
{
    option.write(os);

    return os;
}

bool CommandLine::show_usage()
{
    const size_t start_index = program_path_.find_last_of("\\/");
    size_t end_index = program_path_.rfind('.');
    size_t length = end_index > start_index
        ? end_index - start_index - 1 : program_path_.size() - start_index;

    std::cout << program_path_.substr(start_index + 1, length);
    if (!program_description_.empty()) {
        std::cout << " - " << program_description_;
    }

    std::cout << "\nUsage:\n";

    for (auto&& option : options_) {
        std::cout << "  -" << std::left << std::setw(20) << option.second->get_name()
            << ": " << option.second->get_help_text()
            << " (default: " << std::boolalpha << *(option.second) << ")";

        std::cout << "\n";
    }

    std::cout << "\n";

    return false;
}

void CommandLine::show_options()
{
    for (auto&& option : options_) {
        std::cout
            << std::left << std::setw(20) << option.first
            << ": "
            << std::boolalpha << *(option.second)
            << "\n";
    }
}

bool CommandLine::parse(int argc, char* argv[])
{
    program_path_ = argv[0];
    for (int i = 1; i < argc; i++) {
        if (*argv[i] == '-') {
            std::string option_name = argv[i] + 1;
            if (option_name == "help") {
                show_usage();
                return false;
            }

            auto option = options_.find(option_name);
            if (option == options_.end()) {
                show_usage();
                std::cout << "ERROR: Unknown option '" << argv[i] << "' found in command line.\n";
                return false;
            }

            if (i + 1 < argc && *argv[i + 1] != '-') {
                if (!option->second->set_value(argv[++i])) {
                    show_usage();
                    std::cout << "ERROR: Invalid value '" << argv[i] << "' found for option '" << option_name << "'.\n\n";
                    return false;
                }
            }
            else {
                if (option->second->is_switch()) {
                    option->second->set_value("1");
                }
                else {
                    std::cout << "ERROR: Missing value for option '" << option_name << "'.\n\n";
                    return false;
                }
            }
        }
    }

    //for (auto&& validator : validators_) {
    //    auto option = options_.find(validator.first);
    //    if (option != options_.end()) {
    //        validator.second(*option->second);
    //    }
    //}

    return true;
}

template<>
bool Option<bool>::set_value(const std::string& option_value)
{
    if (option_value.empty()) {
        value_ = true;
    }
    else if (option_value.find_first_not_of("0123456789") == std::string::npos) {
        std::istringstream(option_value) >> value_;
    }
    else {
        std::istringstream(option_value) >> std::boolalpha >> value_;
    }

    return true;
}

//CommandLine& CommandLine::add_validator(const std::string& name, Validator validator)
//{
//    validators_.insert(std::make_pair(name, validator));
//}
