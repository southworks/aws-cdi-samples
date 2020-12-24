#pragma once

namespace CdiTools
{
    namespace Utils
    {
        template <typename T, typename TInserter>
        bool split(const std::string& text, char delimiter, TInserter values)
        {
            std::string token;
            std::istringstream input(text);

            while (std::getline(input, token, delimiter)) {
                if (delimiter != ' ') {
                    token.erase(token.begin(), std::find_if(token.begin(), token.end(), [](unsigned char ch) {
                        return !std::isspace(ch);
                        }));

                    token.erase(std::find_if(token.rbegin(), token.rend(), [](unsigned char ch) {
                        return !std::isspace(ch);
                        }).base(), token.end());
                }

                if (!token.empty()) {
                    T value;
                    std::istringstream parser(token);

                    parser >> value;
                    if (parser.fail() || !parser.eof()) {
                        return false;
                    }

                    *values++ = value;
                }
            }

            return true;
        }
    }
}