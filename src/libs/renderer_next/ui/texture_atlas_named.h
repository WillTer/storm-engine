#pragma once

#include <filesystem>
#include <memory>
#include <unordered_map>
#include <vector>

#include <libs/renderer_next/types.h>

namespace storm
{

class GPUTexture;
class GPUCopyPass;
class GPURenderPass;

class Image2D;

class TextureAtlasNamed final
{
public:
    TextureAtlasNamed(std::filesystem::path const& texture);
    ~TextureAtlasNamed();

    void update(GPUCopyPass const& copy_pass, uint64_t delta_time);

    void add_picture(std::string const& name, storm::FRect const& texture_pos_rect);
    auto get_tex_coords(std::string const& name) const -> storm::FRect;
    auto get_texture() const -> std::shared_ptr<GPUTexture>;

private:
    bool m_need_upload;

    struct UploadData {
        std::vector<char> texture_data;
    } m_upload_data;

    uint32_t m_width;
    uint32_t m_height;

    std::shared_ptr<GPUTexture> m_texture;

    std::unordered_map<std::string, storm::FRect> m_pictures;
};

}  // namespace storm
