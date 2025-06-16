#pragma once

#include <stdexcept>
#include <string>

namespace storm
{
class IniFile;

template <typename T>
struct read_to {
    static T from_ini([[maybe_unused]] IniFile const& ini, [[maybe_unused]] std::string const& section)
    {
        throw std::invalid_argument("Not implemented");
    }
};

template <typename T>
struct convert_to {
    static T from_string(std::string const& s)
    {
        throw std::invalid_argument("Not implemented");
    }
};

template <>
struct convert_to<std::string> {
    static std::string from_string(std::string const& s)
    {
        return s;
    }
};

template <>
struct convert_to<int> {
    static int from_string(std::string const& s)
    {
        return std::stoi(s);
    }
};

template <>
struct convert_to<uint32_t> {
    static uint32_t from_string(std::string const& s)
    {
        return std::stoul(s);
    }
};

template <>
struct convert_to<uint64_t> {
    static uint64_t from_string(std::string const& s)
    {
        return std::stoull(s);
    }
};

template <>
struct convert_to<float> {
    static float from_string(std::string const& s)
    {
        return std::stof(s);
    }
};

template <>
struct convert_to<double> {
    static double from_string(std::string const& s)
    {
        return std::stod(s);
    }
};

template <>
struct convert_to<bool> {
    static bool from_string(std::string const& s)
    {
        return static_cast<bool>(std::stoi(s));
    }
};

}  // namespace storm
