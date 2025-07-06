#pragma once

#include <filesystem>
#include <string>

namespace storm
{

class IConfigLoader;

struct TextureSequenceInfo {
    std::filesystem::path texture_file;

    bool     flip_h;
    bool     flip_v;
    uint32_t time_delay;
    uint32_t width;
    uint32_t height;
    uint32_t h_frames_count;
    uint32_t v_frames_count;
};

namespace texture_sequence
{

TextureSequenceInfo info(IConfigLoader& config_loader, std::string const& name);

}

}  // namespace storm
