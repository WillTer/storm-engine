#include "technique.h"

#include <format>
#include <unordered_map>

#include <libs/config/i_config_loader.h>
#include <libs/config/ini_file.h>

#include "ini_helpers.h"

using namespace storm;
using namespace storm::technique;

template <>
struct storm::convert_to<FillMode> {
    static FillMode from_string(std::string const& s)
    {
        static std::unordered_map<std::string, FillMode> const map {
            {"Fill", FillMode::Fill},
            {"fill", FillMode::Fill},
            {"Line", FillMode::Line},
            {"line", FillMode::Line},
        };

        return map.contains(s) ? map.at(s) : FillMode::Fill;
    }
};

template <>
struct storm::convert_to<Filter> {
    static Filter from_string(std::string const& s)
    {
        static std::unordered_map<std::string, Filter> const map {
            {"Nearest", Filter::Nearest},
            {"nearest", Filter::Nearest},
            {"Linear", Filter::Linear},
            {"linear", Filter::Linear},
        };

        return map.contains(s) ? map.at(s) : Filter::Linear;
    }
};

template <>
struct storm::convert_to<AddressMode> {
    static AddressMode from_string(std::string const& s)
    {
        static std::unordered_map<std::string, AddressMode> const map {
            {"Repeat", AddressMode::Repeat},
            {"repeat", AddressMode::Repeat},
            {"Mirror", AddressMode::Mirror},
            {"mirror", AddressMode::Mirror},
            {"Clamp", AddressMode::Clamp},
            {"clamp", AddressMode::Clamp},
        };

        return map.contains(s) ? map.at(s) : AddressMode::Repeat;
    }
};

template <>
struct storm::convert_to<PrimitiveType> {
    static PrimitiveType from_string(std::string const& s)
    {
        static std::unordered_map<std::string, PrimitiveType> const map {
            {"Triangles", PrimitiveType::Triangles},
            {"triangles", PrimitiveType::Triangles},
            {"Lines", PrimitiveType::Lines},
            {"lines", PrimitiveType::Lines},
        };

        return map.contains(s) ? map.at(s) : PrimitiveType::Triangles;
    }
};

template <>
struct storm::convert_to<BlendOp> {
    static BlendOp from_string(std::string const& s)
    {
        static std::unordered_map<std::string, BlendOp> const map {
            {"Add", BlendOp::Add},
            {"add", BlendOp::Add},
            {"Subtract", BlendOp::Subtract},
            {"subtract", BlendOp::Subtract},
            {"ReverseSubtract", BlendOp::ReverseSubtract},
            {"reverse_subtract", BlendOp::ReverseSubtract},
            {"Min", BlendOp::Min},
            {"min", BlendOp::Min},
            {"Max", BlendOp::Max},
            {"max", BlendOp::Max},
        };

        return map.contains(s) ? map.at(s) : BlendOp::Add;
    }
};

template <>
struct storm::convert_to<BlendFactor> {
    static BlendFactor from_string(std::string const& s)
    {
        static std::unordered_map<std::string, BlendFactor> const map {
            {"One", BlendFactor::One},
            {"one", BlendFactor::One},
            {"Zero", BlendFactor::Zero},
            {"zero", BlendFactor::Zero},
            {"SrcColor", BlendFactor::SrcColor},
            {"src_color", BlendFactor::SrcColor},
            {"OneMinusSrcColor", BlendFactor::OneMinusSrcColor},
            {"one_minus_src_color", BlendFactor::OneMinusSrcColor},
            {"DstColor", BlendFactor::DstColor},
            {"dst_color", BlendFactor::DstColor},
            {"OneMinusDstColor", BlendFactor::OneMinusDstColor},
            {"one_minus_dst_color", BlendFactor::OneMinusDstColor},
            {"SrcAlpha", BlendFactor::SrcAlpha},
            {"src_alpha", BlendFactor::SrcAlpha},
            {"OneMinusSrcAlpha", BlendFactor::OneMinusSrcAlpha},
            {"one_minus_src_alpha", BlendFactor::OneMinusSrcAlpha},
            {"DstAlpha", BlendFactor::DstAlpha},
            {"dst_alpha", BlendFactor::DstAlpha},
            {"OneMinusDstAlpha", BlendFactor::OneMinusDstAlpha},
            {"one_minus_dst_alpha", BlendFactor::OneMinusDstAlpha},
            {"Constant", BlendFactor::Constant},
            {"constant", BlendFactor::Constant},
            {"OneMinusConstantColor", BlendFactor::OneMinusConstantColor},
            {"one_minus_constant_color", BlendFactor::OneMinusConstantColor},
            {"SrcAlphaSaturate", BlendFactor::SrcAlphaSaturate},
            {"src_alpha_saturate", BlendFactor::SrcAlphaSaturate},
        };

        return map.contains(s) ? map.at(s) : BlendFactor::One;
    }
};

template <>
struct storm::read_to<storm::TechniqueInfo> {
    static TechniqueInfo from_ini(IniFile const& ini, std::string const& section)
    {
        auto info = TechniqueInfo {
            .pipeline =
                {
                    .fragment_shader = ini.find_or<std::string>(section, "fragment_shader", {}),
                    .fill_mode       = ini.find_or(section, "fill_mode", FillMode::Fill),
                    .primitive_type  = ini.find_or(section, "primitive_type", PrimitiveType::Triangles),
                    .src_color       = ini.find_or(section, "src_color", BlendFactor::SrcAlpha),
                    .dst_color       = ini.find_or(section, "dst_color", BlendFactor::OneMinusSrcAlpha),
                    .src_alpha       = ini.find_or(section, "src_alpha", BlendFactor::One),
                    .dst_alpha       = ini.find_or(section, "dst_alpha", BlendFactor::One),
                    .color_blend_op  = ini.find_or(section, "color_blend_op", BlendOp::Add),
                    .alpha_blend_op  = ini.find_or(section, "alpha_blend_op", BlendOp::Add),
                },
            .sampler =
                {
                    .min_filter     = ini.find_or(section, "min_filter", Filter::Linear),
                    .mag_filter     = ini.find_or(section, "mag_filter", Filter::Linear),
                    .mipmap_mode    = ini.find_or(section, "mipmap_mode", Filter::Linear),
                    .address_mode_u = ini.find_or(section, "address_mode_u", AddressMode::Repeat),
                    .address_mode_v = ini.find_or(section, "address_mode_v", AddressMode::Repeat),
                    .address_mode_w = ini.find_or(section, "address_mode_w", AddressMode::Repeat),
                },
        };

        return info;
    }
};

namespace storm::technique
{

TechniqueInfo info(IConfigLoader& config_loader, std::filesystem::path const& config_file, std::string const& name)
{
    auto const& config    = config_loader.open_config_cached(config_file, false);
    auto        font_info = read_to<TechniqueInfo>::from_ini(config, name);

    return font_info;
}

}  // namespace storm::technique
