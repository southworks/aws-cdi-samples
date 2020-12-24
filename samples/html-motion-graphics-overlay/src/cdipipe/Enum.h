#pragma once

#include <map>
#include <string>

struct ci_comparator
{
    struct ignore_case_compare
    {
        bool operator() (const unsigned char& c1, const unsigned char& c2) const {
            return tolower(c1) < tolower(c2);
        }
    };

    bool operator() (const std::string& s1, const std::string& s2) const {
        return std::lexicographical_compare(s1.begin(), s1.end(), s2.begin(), s2.end(), ignore_case_compare());
    }
};

template <typename T>
using enum_map = std::map<std::string, T, ci_comparator>;

template <typename T>
const std::string& enum_name(const enum_map<T>& map, T enum_value)
{
    static std::string unknown_enum_{ "Unknown" };

    auto it = std::find_if(map.begin(), map.end(),
        [&](const std::pair<std::string, T>& pair) {
            return pair.second == enum_value;
        });

    return it != map.end() ? it->first : unknown_enum_;
}

template <typename T>
class EnumMap
{
public:
    typedef typename std::map<const std::string, T, ci_comparator> map_type;
    typedef typename map_type::iterator iterator;

    inline iterator begin() noexcept { return map_.begin(); }
    inline iterator end() noexcept { return map_.end(); }

    EnumMap(std::initializer_list<std::pair<const std::string, T>> list)
    {
        map_.insert(list);
    }

    const std::string& get_name(T enum_value)
    {
        static std::string unknown_enum_{ "Unknown" };

        auto it = std::find_if(map_.cbegin(), map_.cend(),
            [&](const std::pair<const std::string, T>& pair) {
                return pair.second == enum_value;
            });

        return it != map_.cend() ? it->first : unknown_enum_;
    }

private:
    std::map<const std::string, T, ci_comparator> map_;
};


