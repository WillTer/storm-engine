#pragma once

#include <memory>
#include <span>
#include <vector>

#include "gpu_copy_pass.h"
#include "sdl_fwd.h"

namespace storm
{

class GPUBuffer
{
public:
    explicit GPUBuffer();
    virtual ~GPUBuffer();

    template <typename T>
    void
    upload(std::shared_ptr<SDL_GPUDevice> const& device, std::shared_ptr<SDL_GPUCopyPass> const& copy_pass, std::span<T> const& data) const
    {
        upload(device, copy_pass, data.data(), static_cast<uint32_t>(data.size() * sizeof(T)));
    }

    template <typename T>
    void update(
        std::shared_ptr<SDL_GPUDevice> const&   device,
        std::shared_ptr<SDL_GPUCopyPass> const& copy_pass,
        std::vector<BufferUpdateInfo> const&    update_info,
        std::span<T> const&                     data,
        size_t                                  stride) const
    {
        update(device, copy_pass, update_info, data.data(), stride);
    }

protected:
    std::shared_ptr<SDL_GPUBuffer> m_buffer = nullptr;

private:
    void upload(
        std::shared_ptr<SDL_GPUDevice> const&   device,
        std::shared_ptr<SDL_GPUCopyPass> const& copy_pass,
        void const*                             data,
        uint32_t                                data_size) const;

    void update(
        std::shared_ptr<SDL_GPUDevice> const&   device,
        std::shared_ptr<SDL_GPUCopyPass> const& copy_pass,
        std::vector<BufferUpdateInfo> const&    update_info,
        void const*                             data,
        size_t                                  stride) const;
};

}  // namespace storm
