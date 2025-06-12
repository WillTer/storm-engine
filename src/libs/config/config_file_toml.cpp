#include "config_file_toml.h"

#include <libs/core/core.h>
#include <toml.hpp>

using namespace storm;

namespace
{

ConfigTable              convert_toml_table(toml::value const& table);
std::vector<ConfigValue> convert_toml_array(toml::value const& arr);

ConfigValue convert_toml_value(toml::value const& value)
{
    if (value.is_boolean()) { return ConfigValue(value.as_boolean()); }
    if (value.is_integer()) { return ConfigValue(value.as_integer()); }
    if (value.is_floating()) { return ConfigValue(value.as_floating()); }
    if (value.is_string()) { return ConfigValue(value.as_string()); }

    if (value.is_table()) { return ConfigValue(convert_toml_table(value)); }
    if (value.is_array()) { return ConfigValue(convert_toml_array(value)); }

    throw std::runtime_error("Unsupported toml type");
}

ConfigTable convert_toml_table(toml::value const& table)
{
    ConfigTable result = {};

    for (auto const& [key, value]: table.as_table()) {
        result.emplace(key, convert_toml_value(value));
    }

    return result;
}

std::vector<ConfigValue> convert_toml_array(toml::value const& arr)
{
    std::vector<ConfigValue> result = {};

    for (auto const& el: arr.as_array()) {
        result.emplace_back(convert_toml_value(el));
    }

    return result;
}

}  // namespace

ConfigFileToml::ConfigFileToml(IFileService& file_service, std::filesystem::path const& path)
{
    auto stream = file_service.open_file<std::ifstream>(path, std::ios::binary);

    try {
        auto const toml_table = toml::parse(stream, path.string());
        m_table               = convert_toml_table(toml_table);
    } catch (toml::exception const& e) {
        core.Trace("%s", e.what());
    }
}
