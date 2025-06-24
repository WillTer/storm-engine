#pragma once

#include <expected>
#include <filesystem>
#include <fstream>

namespace storm::asset_loader
{

enum class Error {
    FileNotFound,
    ExtensionNotSupported,
    NotImplemented,
};

template <typename T>
std::expected<T, Error> from_file(std::filesystem::path const& /*path*/)
{
    return std::unexpected(Error::NotImplemented);
}

template <>
inline std::expected<std::vector<char>, Error> from_file(std::filesystem::path const& path)
{
    auto file = std::ifstream(path, std::ios::binary);

    file.seekg(0, std::ios::end);
    size_t const file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> content(file_size);
    file.read(content.data(), file_size);

    return content;
}

}  // namespace storm::asset_loader
