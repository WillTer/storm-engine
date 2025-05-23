#include "base_data_stream.h"

#include <fstream>
#include <vector>

using namespace storm::audio;

BaseDataStream::BaseDataStream() = default;

BaseDataStream::~BaseDataStream() = default;

bool BaseDataStream::load_file(std::filesystem::path const& file_path)
{
    auto file = std::ifstream(file_path, std::ios::binary);
    if (!file) { return false; }

    file.unsetf(std::ios::skipws);

    file.seekg(0, std::ios::end);
    auto const sz = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> file_data = {};
    file_data.reserve(sz);
    file_data.insert(file_data.end(), std::istream_iterator<uint8_t>(file), std::istream_iterator<uint8_t>());

    return load_memory(file_data);
}
