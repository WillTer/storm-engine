#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

#include <libs/renderer_next/types.h>

namespace storm
{

class IConfigLoader;

namespace technique
{
enum class FillMode { Fill, Line };
enum class Filter { Nearest, Linear };
enum class AddressMode { Clamp, Repeat, Mirror };
enum class PrimitiveType { Lines, Triangles };
enum class BlendOp { Add, Subtract, ReverseSubtract, Min, Max };
enum class BlendFactor {
    One,
    Zero,
    SrcColor,
    OneMinusSrcColor,
    DstColor,
    OneMinusDstColor,
    SrcAlpha,
    OneMinusSrcAlpha,
    DstAlpha,
    OneMinusDstAlpha,
    Constant,
    OneMinusConstantColor,
    SrcAlphaSaturate
};
}  // namespace technique

struct PipelineInfo {
    std::string              fragment_shader;
    technique::FillMode      fill_mode;
    technique::PrimitiveType primitive_type;
    technique::BlendFactor   src_color;
    technique::BlendFactor   dst_color;
    technique::BlendFactor   src_alpha;
    technique::BlendFactor   dst_alpha;
    technique::BlendOp       color_blend_op;
    technique::BlendOp       alpha_blend_op;
};

struct SamplerInfo {
    technique::Filter      min_filter;
    technique::Filter      mag_filter;
    technique::Filter      mipmap_mode;
    technique::AddressMode address_mode_u;
    technique::AddressMode address_mode_v;
    technique::AddressMode address_mode_w;
};

struct TechniqueInfo {
    std::string  name;
    PipelineInfo pipeline;
    SamplerInfo  sampler;
};

namespace technique
{

TechniqueInfo info(IConfigLoader& config_loader, std::filesystem::path const& config_file, std::string const& name);

}

}  // namespace storm
