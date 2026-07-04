#include "image_2d.h"

#include <algorithm>
#include <cassert>

#include <libs/asset_server/asset_server.h>
#include <libs/core/core.h>
#include <libs/renderer_next/impl_sdl/gpu_command_buffer.h>
#include <libs/renderer_next/impl_sdl/gpu_index_buffer.h>
#include <libs/renderer_next/impl_sdl/gpu_texture.h>
#include <libs/renderer_next/impl_sdl/gpu_vertex_buffer.h>
#include <libs/renderer_next/impl_sdl/renderer_sdl.h>
#include <libs/renderer_next/pipeline_names.h>
#include <shaders/ui/image_2d.h>

using namespace storm;
using namespace hlslpp;

using ImageVertex = shaders::image_2d::VertexInput;

namespace
{

auto const SQUARE_INDICES = std::vector<uint32_t> {0, 1, 2, 0, 2, 3};

}  // namespace

Image2D::Image2D(std::filesystem::path const& texture, std::shared_ptr<GPUSampler> const& sampler /*= nullptr*/)
{
    auto const& renderer = core->get<RendererService>();
    m_texture            = renderer->create_texture(texture.string(), sampler);

    initialize();
}

Image2D::Image2D(std::shared_ptr<GPUTexture> const& external_texture) : m_texture(external_texture)
{
    initialize();
}

Image2D::~Image2D() = default;

void Image2D::update(GPUCopyPass const& copy_pass, uint64_t /*delta_time*/)
{
    if (m_is_dirty) {
        auto const update_info = std::vector(4, BufferUpdateInfo {.offset = offsetof(ImageVertex, uv), .size = sizeof(ImageVertex::uv)});
        copy_pass.update_buffer(*m_vertex_buffer, update_info, m_texture_uv, sizeof(ImageVertex));

        if (m_texture) {
            auto const [width, height] = m_texture->get_dimensions();

            float min_x = std::numeric_limits<float>::max();
            float max_x = std::numeric_limits<float>::min();
            float min_y = std::numeric_limits<float>::max();
            float max_y = std::numeric_limits<float>::min();

            for (size_t i = 0; i < m_texture_uv.size(); ++i) {
                min_x = std::min<float>(m_texture_uv[i].x, min_x);
                max_x = std::max<float>(m_texture_uv[i].x, max_x);
                min_y = std::min<float>(m_texture_uv[i].y, min_y);
                max_y = std::max<float>(m_texture_uv[i].y, max_y);
            }

            m_rect   = {0.0F, 0.0F, width * (max_x - min_x), height * (max_y - min_y)};
            m_width  = static_cast<uint32_t>(m_rect.width());
            m_height = static_cast<uint32_t>(m_rect.height());
        }

        m_is_dirty = false;
    }
}

void Image2D::draw(GPURenderPass const& render_pass) const
{
    if (!m_texture) {
        return;
    }

    render_pass.bind(*m_pipeline);
    render_pass.bind(*m_index_buffer);
    render_pass.bind(*m_vertex_buffer);

    render_pass.push_vertex_uniform_data(0, m_vertex_ubo);
    render_pass.push_fragment_uniform_data(0, m_fragment_ubo);
    render_pass.bind(*m_texture);
    render_pass.draw(*m_index_buffer);
}

void Image2D::set_pipeline(entt::hashed_string const& name)
{
    auto const& renderer = core->get<RendererService>();

    m_pipeline = renderer->create_pipeline(name);
}

void Image2D::initialize()
{
    auto const& renderer = core->get<RendererService>();

    m_pipeline = renderer->create_pipeline(IMAGE_2D_PIPELINE);

    auto const vertex_data = std::vector<ImageVertex> {
        ImageVertex {{0.0F, 0.0F}, {0.0F, 0.0F}},
        ImageVertex {{1.0F, 0.0F}, {1.0F, 0.0F}},
        ImageVertex {{1.0F, 1.0F}, {1.0F, 1.0F}},
        ImageVertex {{0.0F, 1.0F}, {0.0F, 1.0F}},
    };

    m_vertex_buffer = renderer->create_vertex_buffer(vertex_data);
    m_index_buffer  = renderer->create_index_buffer(SQUARE_INDICES);

    m_fragment_ubo.color = float4(1.0F);
}

void Image2D::set_diffuse_color(storm::Color const& color)
{
    auto const [r, g, b, a] = color.normalize();
    m_fragment_ubo.color    = float4(r, g, b, a);
}

void Image2D::set_uv(storm::FRect const& texture_uv)
{
    m_texture_uv = {
        {texture_uv.left, texture_uv.top},
        {texture_uv.right, texture_uv.top},
        {texture_uv.right, texture_uv.bottom},
        {texture_uv.left, texture_uv.bottom},
    };

    m_is_dirty = true;
}

void Image2D::set_uv_full(std::array<float2, 4> const& texture_uv)
{
    m_texture_uv.clear();
    m_texture_uv.insert(m_texture_uv.end(), texture_uv.begin(), texture_uv.end());
    m_is_dirty = true;
}
