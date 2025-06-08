#pragma once

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace storm
{

class ConfigValue final
{
    template <typename T>
    T const& as_internal() const
    {
        if (!std::holds_alternative<T>(m_value)) { throw std::runtime_error("Invalid config value type"); }
        return std::get<T>(m_value);
    }

public:
    using Table = std::unordered_map<std::string, ConfigValue>;

    ConfigValue() = default;

    template <typename T>
    explicit ConfigValue(T val) : m_value {val}
    {
    }

    template <typename T>
    T as() const
    {
        if constexpr (std::is_same_v<T, bool>) {
            return as_internal<bool>();
        } else if constexpr (std::is_integral_v<T>) {
            return static_cast<T>(as_internal<intmax_t>());
        } else if constexpr (std::is_floating_point_v<T>) {
            return static_cast<T>(as_internal<double>());
        } else if constexpr (std::is_convertible_v<std::string, T>) {
            return to_string();
        } else if constexpr (std::is_same_v<Table, T>) {
            return as_internal<Table>();
        } else if constexpr (std::is_same_v<std::vector<ConfigValue>, T>) {
            return as_internal<std::vector<ConfigValue>>();
        } else {
            static_assert(false, "Type is unsupported");
        }
    }

    std::string to_string() const
    {
        if (std::holds_alternative<bool>(m_value)) { return std::get<bool>(m_value) ? "true" : "false"; }
        if (std::holds_alternative<intmax_t>(m_value)) { return std::to_string(std::get<intmax_t>(m_value)); }
        if (std::holds_alternative<double>(m_value)) { return std::to_string(std::get<double>(m_value)); }
        if (std::holds_alternative<std::string>(m_value)) { return std::get<std::string>(m_value); }

        return "";
    }

    template <typename T>
    std::vector<T> as_array() const
    {
        auto const arr = as_internal<std::vector<ConfigValue>>();

        std::vector<T> vec = {};
        vec.reserve(arr.size());

        for (auto const& val: arr) {
            vec.emplace_back(val.as<T>());
        }

        return vec;
    }

    Table const& as_table() const
    {
        return as_internal<Table>();
    }

    bool is_bool() const
    {
        return std::holds_alternative<bool>(m_value);
    }

    bool is_int() const
    {
        return std::holds_alternative<intmax_t>(m_value);
    }

    bool is_float() const
    {
        return std::holds_alternative<double>(m_value);
    }

    bool is_string() const
    {
        return std::holds_alternative<std::string>(m_value);
    }

    bool is_table() const
    {
        return std::holds_alternative<Table>(m_value);
    }

    bool is_array() const
    {
        return std::holds_alternative<std::vector<ConfigValue>>(m_value);
    }

private:
    std::variant<bool, intmax_t, double, std::string, Table, std::vector<ConfigValue>> m_value;
};

using ConfigTable = ConfigValue::Table;

namespace config
{

static inline bool find(ConfigTable const& table, std::string const& path, ConfigValue& value)
{
    if (path.empty()) { return false; }

    size_t offset        = 0;
    auto   delimiter_pos = path.find_first_of('.', offset);
    auto   key           = path.substr(offset, delimiter_pos);

    auto section = std::cref(table);
    while (delimiter_pos != std::string::npos) {
        // Trying to get next table if there are dots in path
        if (!section.get().contains(key) || !section.get().at(key).is_table()) { return false; }
        section = section.get().at(key).as_table();

        offset += delimiter_pos + 1;
        delimiter_pos = path.find_first_of('.', offset);
        key           = path.substr(offset, delimiter_pos);
    }

    // Trying to get value from the last part of the path
    if (!section.get().contains(key)) { return false; }

    value = section.get().at(key);
    return true;
}

static inline std::optional<ConfigValue> try_find_value(ConfigTable const& table, std::string const& path)
{
    if (ConfigValue value = {}; find(table, path, value)) { return value; }
    return std::nullopt;
}

static inline ConfigValue find_value(ConfigTable const& table, std::string const& path)
{
    if (auto const value = try_find_value(table, path); value.has_value()) { return value.value(); }
    throw std::runtime_error("No config key \"" + path + "\"");
}

template <typename T>
static inline T find(ConfigTable const& table, std::string const& path)
{
    if (auto const value = try_find_value(table, path); value.has_value()) { return value.value().as<T>(); }
    throw std::runtime_error("No config key \"" + path + "\"");
}

template <typename T>
static inline T find(ConfigTable const& table, std::string const& path, T const& default_val)
{
    if (auto const value = try_find_value(table, path); value.has_value()) { return value.value().as<T>(); }
    return default_val;
}

template <typename T>
static inline std::optional<T> try_find(ConfigTable const& table, std::string const& path)
{
    if (auto const value = try_find_value(table, path); value.has_value()) { return value.value().as<T>(); }
    return std::nullopt;
}

}  // namespace config

class ConfigFile
{
public:
    virtual ~ConfigFile() = default;

    template <typename T>
    T get(std::string const& path, std::string const& key) const
    {
        auto const full_path = path.empty() ? key : path + "." + key;
        return config::find<T>(m_table, full_path);
    }

    template <typename T>
    T get(std::string const& path, std::string const& key, T const& default_val) const
    {
        auto const full_path = path.empty() ? key : path + "." + key;
        return config::find<T>(m_table, full_path, default_val);
    }

    template <typename T>
    std::optional<T> try_get(std::string const& path, std::string const& key) const
    {
        auto const full_path = path.empty() ? key : path + "." + key;
        return config::try_find<T>(m_table, full_path);
    }

    ConfigTable const& as_table() const
    {
        return m_table;
    }

protected:
    ConfigTable m_table;
};

}  // namespace storm
