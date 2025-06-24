#pragma once

#include <expected>
#include <filesystem>

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

}  // namespace storm::asset_loader
