#pragma once

#include <filesystem>
#include <memory>

namespace storm
{

enum class AudioState
{
    None,
    Initial,
    Playing,
    Paused,
    Stopped,
};

class OggPlayer
{
  public:
    OggPlayer();
    ~OggPlayer();

    bool Init(size_t samplesBufferSize = 1024);

    bool OpenFileStream(const std::filesystem::path &path);

    void Play();
    void Pause();
    void Stop();

    void Update();

    AudioState GetState() const;

    unsigned GetSourceId() const;

  private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace storm
