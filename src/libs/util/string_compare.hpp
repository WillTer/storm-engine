#pragma once

#include <algorithm>
#include <cctype>
#include <string>

#include "istring.hpp"

namespace storm
{
namespace detail
{
struct is_iequal {
    template <typename T1, typename T2 = T1>
    bool operator()(const T1& first, const T2& second) const
    {
        return std::toupper(first) == std::toupper(second);
    }
};

}  // namespace detail

template <typename Range1T, typename Range2T = Range1T>
bool iEquals(Range1T const& first, Range2T const& second, size_t const count)
{
    detail::is_iequal comp;

    auto const& first_normalized  = std::is_pointer<Range1T>::value ? std::string_view(first) : first;
    auto const& second_normalized = std::is_pointer<Range1T>::value ? std::string_view(second) : second;

    auto const first_begin  = std::begin(first_normalized);
    auto const second_begin = std::begin(second_normalized);

    auto const first_end  = std::end(first_normalized);
    auto const second_end = std::end(second_normalized);

    auto const first_length  = std::distance(first_begin, first_end);
    auto const second_length = std::distance(second_begin, second_end);
    if (first_length < count || second_length < count) {
        if (first_length != second_length) {
            return false;
        } else {
            return std::equal(first_begin, first_begin + first_length, second_begin, second_begin + second_length, comp);
        }
    } else {
        return std::equal(first_begin, first_begin + count, second_begin, second_begin + count, comp);
    }
}

template <typename Range1T, typename Range2T = Range1T>
bool iEquals(Range1T const& first, Range2T const& second)
{
    auto const& first_normalized  = std::is_pointer<Range1T>::value ? std::string_view(first) : first;
    auto const& second_normalized = std::is_pointer<Range1T>::value ? std::string_view(second) : second;

    return traits_cast<ichar_traits<char>>(first_normalized) == traits_cast<ichar_traits<char>>(second_normalized);
}

template <typename Range1T, typename Range2T = Range1T>
bool iLess(Range1T const& first, Range2T const& second)
{
    auto const& first_normalized  = std::is_pointer<Range1T>::value ? std::string_view(first) : first;
    auto const& second_normalized = std::is_pointer<Range1T>::value ? std::string_view(second) : second;

    return traits_cast<ichar_traits<char>>(first_normalized) < traits_cast<ichar_traits<char>>(second_normalized);
}

template <typename Range1T, typename Range2T = Range1T>
bool iLessOrEqual(Range1T const& first, Range2T const& second)
{
    auto const& first_normalized  = std::is_pointer<Range1T>::value ? std::string_view(first) : first;
    auto const& second_normalized = std::is_pointer<Range1T>::value ? std::string_view(second) : second;

    return traits_cast<ichar_traits<char>>(first_normalized) <= traits_cast<ichar_traits<char>>(second_normalized);
}

template <typename Range1T, typename Range2T = Range1T>
bool iGreater(Range1T const& first, Range2T const& second)
{
    return !iLessOrEqual(first, second);
}

template <typename Range1T, typename Range2T = Range1T>
bool iGreaterOrEqual(Range1T const& first, Range2T const& second)
{
    return !iLess(first, second);
}

class iStrHasher
{
public:
    size_t operator()(std::string const& key) const
    {
        std::string lower_copy = key;
        std::transform(lower_copy.begin(), lower_copy.end(), lower_copy.begin(), ::tolower);
        return inner_hasher_(lower_copy);
    }

private:
    std::hash<std::string> inner_hasher_;
};

struct iStrComparator {
    bool operator()(std::string const& left, std::string const& right) const
    {
        return iEquals(left, right);
    }
};
}  // namespace storm
