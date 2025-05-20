#pragma once

// #include <filesystem>
// #include <memory>

namespace storm
{

enum class AudioState {
    None,
    Initial,
    Playing,
    Paused,
    Stopped,
};

// class OggPlayer
// {
// public:
//     OggPlayer();
//     ~OggPlayer();

//     bool init(size_t samples_buffer_size = 1024);

//     bool open_file_stream(std::filesystem::path const& path);

//     void play();
//     void pause();
//     void stop();

//     void update();

//     AudioState get_state() const;

//     unsigned get_source_id() const;

// private:
//     struct Impl;
//     std::unique_ptr<Impl> m_impl;
// };

}  // namespace storm
