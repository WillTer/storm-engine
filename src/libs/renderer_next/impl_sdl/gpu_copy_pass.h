#pragma once

#include <memory>
#include <span>

#include "concepts.h"
#include "sdl_fwd.h"

namespace storm
{

class GPUCopyPass final
{
public:
    GPUCopyPass(std::shared_ptr<SDL_GPUDevice> const& device, std::shared_ptr<SDL_GPUCommandBuffer> const& cmd_buffer);
    ~GPUCopyPass();

    template <typename V>
    void upload(can_upload<V> auto const& buffer, std::span<V> const& data) const
    {
        buffer.upload(m_device, m_pass, data);
    }

    template <typename V>
    void update_buffer(
        can_update<V> auto const&            buffer,
        std::vector<BufferUpdateInfo> const& update_info,
        std::span<V> const&                  data,
        size_t const                         stride) const
    {
        buffer.update(m_device, m_pass, update_info, data, stride);
    }

private:
    std::shared_ptr<SDL_GPUDevice>   m_device = nullptr;
    std::shared_ptr<SDL_GPUCopyPass> m_pass   = nullptr;
};

}  // namespace storm
