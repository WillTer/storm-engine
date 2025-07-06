#pragma once

#include <memory>
#include <vector>

#include "gpu_copy_pass.h"
#include "sdl_fwd.h"

namespace storm
{

class GPUBuffer
{
public:
    GPUBuffer(std::shared_ptr<SDL_GPUDevice> const& device, void const* data, uint32_t data_size);
    virtual ~GPUBuffer();

    void upload(std::shared_ptr<SDL_GPUCopyPass> const& copy_pass);

    template <typename T>
    void update(
        std::shared_ptr<SDL_GPUDevice> const&   device,
        std::shared_ptr<SDL_GPUCopyPass> const& copy_pass,
        std::vector<BufferUpdateInfo> const&    update_info,
        std::vector<T> const&                   data,
        size_t                                  stride) const
    {
        update(device, copy_pass, update_info, data.data(), stride);
    }

protected:
    std::shared_ptr<SDL_GPUBuffer> m_buffer = nullptr;

private:
    void update(
        std::shared_ptr<SDL_GPUDevice> const&   device,
        std::shared_ptr<SDL_GPUCopyPass> const& copy_pass,
        std::vector<BufferUpdateInfo> const&    update_info,
        void const*                             data,
        size_t                                  stride) const;

    std::shared_ptr<SDL_GPUTransferBuffer> m_transfer_buffer;
    uint32_t                               m_transfer_size;
};

}  // namespace storm
