// #include "ogg_player.h"

// #include <array>

// #include <AL/al.h>
// #include <AL/alc.h>
// #include <libs/core/core.h>

// #include "openal/al_utils.h"

// #include "stb_vorbis.c"

// using namespace storm;

// #define CHECK_INITIALIZED() \
//     do { \
//         if (!m_is_initialized) { \
//             std::stringstream ss; \
//             ss << "[" << __FILE__ << ":" << __LINE__ << "] OGG Player is not initialized"; \
//             throw std::runtime_error(ss.str()); \
//         } \
//     } while (false)
// #define CHECK_ACTIVE_FILE_OPENED() \
//     do { \
//         if (!m_active_file.has_value()) { \
//             std::stringstream ss; \
//             ss << "[" << __FILE__ << ":" << __LINE__ << "] No file streams opened"; \
//             throw std::runtime_error(ss.str()); \
//         } \
//     } while (false)

// namespace
// {

// constexpr size_t AUDIO_STREAM_SOURCE_COUNT = 2;  // 2 sources: one currently playing and the second is loading
// constexpr size_t AUDIO_STREAM_BUFFER_COUNT = 4;

// class OggFile
// {
//     enum class UpdateStreamResult { Updated, NoData };

// public:
//     OggFile(
//         std::filesystem::path const&                           path,
//         unsigned                                               source,
//         std::array<unsigned, AUDIO_STREAM_BUFFER_COUNT> const& buffers,
//         size_t                                                 bufferSampleCountByChannel)
//         : m_source {source}
//     {
//         std::ifstream file(path, std::ios::binary);
//         if (!file) { throw std::runtime_error("Unable to open OGG file: " + path.string()); }

//         m_file_data = std::vector((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

//         int err  = 0;
//         m_stream = stb_vorbis_open_memory(
//             reinterpret_cast<unsigned char const*>(m_file_data.data()), static_cast<int>(m_file_data.size()), &err, nullptr);
//         if (m_stream == nullptr) {
//             throw std::runtime_error("Unable to decode OGG file " + path.string() + ", code: " + std::to_string(err));
//         }

//         auto const info = stb_vorbis_get_info(m_stream);

//         m_channels    = info.channels;
//         m_sample_rate = info.sample_rate;
//         m_format      = m_channels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
//         m_offset      = 0;
//         m_intermediate_buffer.resize(bufferSampleCountByChannel * info.channels);

//         static_assert(sizeof(buffers) == sizeof(m_buffers));
//         std::memcpy(m_buffers.data(), buffers.data(), m_buffers.size() * sizeof(unsigned));

//         for (auto& buffer: m_buffers) {
//             update_stream(buffer);
//         }

//         alSourceQueueBuffers(m_source, static_cast<int>(m_buffers.size()), m_buffers.data());
//     }

//     ~OggFile()
//     {
//         stb_vorbis_close(m_stream);
//     }

//     void play()
//     {
//         if (get_state() == AudioState::Playing) { return; }

//         alSourcePlay(m_source);
//     }

//     void pause()
//     {
//         if (get_state() != AudioState::Playing) { return; }

//         alSourcePause(m_source);
//     }

//     void stop()
//     {
//         if (get_state() != AudioState::Playing && get_state() != AudioState::Paused) { return; }

//         alSourceStop(m_source);
//     }

//     void set_looped(bool isLooped)
//     {
//         m_is_playback_looped = isLooped;
//     }

//     AudioState get_state() const
//     {
//         ALint state = 0;
//         alGetSourcei(m_source, AL_SOURCE_STATE, &state);

//         switch (state) {
//         case AL_PLAYING: return AudioState::Playing;
//         case AL_PAUSED: return AudioState::Paused;
//         case AL_STOPPED: return AudioState::Stopped;
//         default: break;
//         }

//         return AudioState::None;
//     }

//     void update()
//     {
//         if (get_state() != AudioState::Playing) { return; }

//         ALint processed = 0;
//         alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &processed);
//         for (ALint i = 0; i < processed; ++i) {
//             ALuint buffer = 0;
//             alSourceUnqueueBuffers(m_source, 1, &buffer);

//             auto res = update_stream(buffer);
//             if (res == UpdateStreamResult::NoData && m_is_playback_looped) {
//                 stb_vorbis_seek_start(m_stream);
//                 m_offset = 0;
//                 res      = update_stream(buffer);
//             }

//             if (res == UpdateStreamResult::Updated) { alSourceQueueBuffers(m_source, 1, &buffer); }
//         }
//     }

// private:
//     UpdateStreamResult update_stream(unsigned buffer)
//     {
//         auto& pcm = m_intermediate_buffer;
//         std::memset(pcm.data(), 0, pcm.size() * sizeof(short));

//         size_t samplesCount = 0;
//         while (samplesCount < pcm.size()) {
//             int const converted = stb_vorbis_get_samples_short_interleaved(
//                 m_stream, m_channels, pcm.data() + samplesCount, static_cast<int>(pcm.size() - samplesCount));
//             if (converted == 0) { break; }

//             samplesCount += converted * m_channels;
//         }

//         if (samplesCount == 0) { return UpdateStreamResult::NoData; }

//         m_offset += samplesCount;

//         alBufferData(buffer, m_format, pcm.data(), samplesCount * sizeof(ALshort), m_sample_rate);

//         return UpdateStreamResult::Updated;
//     }

// private:
//     stb_vorbis*        m_stream;
//     std::vector<char>  m_file_data;
//     std::vector<short> m_intermediate_buffer;  // Buffer for transferring data from decoder to OpenAL
//     size_t             m_offset;

//     int m_format;
//     int m_channels;
//     int m_sample_rate;

//     bool m_is_playback_looped;

//     unsigned                                        m_source;
//     std::array<unsigned, AUDIO_STREAM_BUFFER_COUNT> m_buffers;
// };

// }  // namespace

// struct OggPlayer::Impl {
// public:
//     Impl() = default;

//     ~Impl()
//     {
//         if (m_is_initialized) {
//             for (auto& file: m_active_sounds) {
//                 alDeleteSources(1, &file.source);  // Delete source to free buffers
//                 alDeleteBuffers(static_cast<int>(file.buffers.size()), file.buffers.data());
//             }
//         }
//     }

//     void init(size_t samples_buffer_size)
//     {
//         if (m_is_initialized) { return; }

//         alGetError();  // clear error code
//         for (auto& file: m_active_sounds) {
//             alGenBuffers(static_cast<int>(file.buffers.size()), file.buffers.data());
//             alGenSources(1, &file.source);
//         }

//         m_buffer_sample_count_by_channel = samples_buffer_size;
//         m_is_initialized                 = true;
//     }

//     void open_file_stream(std::filesystem::path const& path)
//     {
//         CHECK_INITIALIZED();

//         if (m_active_sounds[m_active_sound].file) {
//             m_active_sounds[m_active_sound].file->stop();
//             alSourcei(m_active_sounds[m_active_sound].source, AL_BUFFER, 0);  // Detach all queued buffers
//         }

//         m_active_sound                       = !m_active_sound;
//         m_active_sounds[m_active_sound].file = std::make_unique<OggFile>(
//             path, m_active_sounds[m_active_sound].source, m_active_sounds[m_active_sound].buffers, m_buffer_sample_count_by_channel);
//         m_active_sounds[m_active_sound].file->set_looped(true);
//     }

//     void play()
//     {
//         CHECK_INITIALIZED();

//         if (m_active_sounds[m_active_sound].file) { m_active_sounds[m_active_sound].file->play(); }
//     }

//     void pause()
//     {
//         CHECK_INITIALIZED();

//         if (m_active_sounds[m_active_sound].file) { m_active_sounds[m_active_sound].file->pause(); }
//     }

//     void stop()
//     {
//         CHECK_INITIALIZED();

//         if (m_active_sounds[m_active_sound].file) { m_active_sounds[m_active_sound].file->stop(); }
//     }

//     void update()
//     {
//         CHECK_INITIALIZED();

//         if (m_active_sounds[m_active_sound].file) { m_active_sounds[m_active_sound].file->update(); }
//     }

//     AudioState get_state() const
//     {
//         CHECK_INITIALIZED();

//         if (m_active_sounds[m_active_sound].file) { return m_active_sounds[m_active_sound].file->get_state(); }

//         return AudioState::None;
//     }

//     unsigned get_source_id() const
//     {
//         CHECK_INITIALIZED();

//         return m_active_sounds[m_active_sound].source;
//     }

// private:
//     bool   m_is_initialized;
//     size_t m_buffer_sample_count_by_channel;
//     size_t m_active_sound;

//     struct Sound {
//         std::unique_ptr<OggFile> file;

//         unsigned                                        source;
//         std::array<unsigned, AUDIO_STREAM_BUFFER_COUNT> buffers;
//     };

//     std::array<Sound, AUDIO_STREAM_SOURCE_COUNT> m_active_sounds;
// };

// OggPlayer::OggPlayer() : m_impl(std::make_unique<Impl>()) {}

// OggPlayer::~OggPlayer() = default;

// bool OggPlayer::init(size_t samplesBufferSize /*= 1024*/)
// try {
//     m_impl->init(samplesBufferSize);
//     return true;
// } catch (std::runtime_error const& e) {
//     core.Trace("%s\n", e.what());
//     return false;
// }

// bool OggPlayer::open_file_stream(std::filesystem::path const& path)
// try {
//     m_impl->open_file_stream(path);
//     return true;
// } catch (std::runtime_error const& e) {
//     core.Trace("%s\n", e.what());
//     return false;
// }

// void OggPlayer::play()
// try {
//     m_impl->play();
// } catch (std::runtime_error const& e) {
//     core.Trace("%s\n", e.what());
// }

// void OggPlayer::pause()
// try {
//     m_impl->pause();
// } catch (std::runtime_error const& e) {
//     core.Trace("%s\n", e.what());
// }

// void OggPlayer::stop()
// try {
//     m_impl->stop();
// } catch (std::runtime_error const& e) {
//     core.Trace("%s\n", e.what());
// }

// void OggPlayer::update()
// try {
//     m_impl->update();
// } catch (std::runtime_error const& e) {
//     core.Trace("%s\n", e.what());
// }

// AudioState OggPlayer::get_state() const
// try {
//     return m_impl->get_state();
// } catch (std::runtime_error const& e) {
//     core.Trace("%s\n", e.what());
//     return AudioState::None;
// }

// unsigned OggPlayer::get_source_id() const
// try {
//     return m_impl->get_source_id();
// } catch (std::runtime_error const& e) {
//     core.Trace("%s\n", e.what());
//     return 0;
// }
