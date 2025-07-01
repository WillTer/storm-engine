#pragma once

#include <filesystem>
#include <memory>
#include <unordered_map>

#include <libs/renderer_next/types.h>

namespace storm
{

class GPUTexture;
class GPUCopyPass;
class GPURenderPass;

class Picture;

class TextureAtlasNamed final
{
public:
    TextureAtlasNamed(GPUCopyPass const& copy_pass, std::filesystem::path const& texture);
    ~TextureAtlasNamed();

    void add_picture(GPUCopyPass const& copy_pass, std::string const& name, storm::FRect const& texture_pos_rect);
    auto get_picture(std::string const& name) -> std::shared_ptr<Picture>;

private:
    uint32_t m_width;
    uint32_t m_height;

    std::shared_ptr<GPUTexture> m_texture;

    std::unordered_map<std::string, std::shared_ptr<Picture>> m_pictures;
};

}  // namespace storm
