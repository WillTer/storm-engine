#pragma once

#include <cstdint>
#include <memory>
#include <optional>

#include "sdl_fwd.h"

namespace storm
{

struct TextureAsset;
struct ColorTargetInfo;

class GPUSampler final
{
public:
    enum class Filter { Nearest, Linear };
    enum class AddressMode { Repeat, MirroredRepeat, Clamp };

    struct Info {
        Filter               min_filter;
        Filter               mag_filter;
        Filter               mipmap_filter;
        AddressMode          address_mode;
        std::optional<float> max_anisotropy;
    };

    GPUSampler(std::shared_ptr<SDL_GPUDevice> const& device, Info info);
    ~GPUSampler();

    operator SDL_GPUSampler*() const;

private:
    std::shared_ptr<SDL_GPUSampler> m_sampler = nullptr;
};

}  // namespace storm
