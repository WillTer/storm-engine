#pragma once

#include <expected>
#include <filesystem>
#include <stdexcept>

namespace storm::asset_loader
{

enum class Error {
    FileNotFound,
    ExtensionNotSupported,
};

template <typename T>
std::expected<T, Error> from_file(std::filesystem::path const& path)
{
    throw std::runtime_error("Not implemented");
}

}  // namespace storm::asset_loader
