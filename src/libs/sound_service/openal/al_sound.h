#pragma once

#include <memory>

#include <libs/sound_service/i_audio_backend.h>

namespace storm::audio
{

class ALChannel;

class ALSound: virtual public ISound
{
public:
    ALSound(std::unique_ptr<IDataStream>&& stream, Flags flags);
    ~ALSound() override;

    Flags get_flags() const override;

private:
    friend class ALChannel;

    void bind_buffers_to_source(unsigned source, bool is_looping);
    void unbind_source(unsigned source);

    void set_looping(unsigned source, bool is_looping);

    int get_channels() const;

    bool push_next_data(unsigned buffer, bool is_looping) const;
    void reset_buffers();

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace storm::audio
