#include "ogg_player.h"

#include <array>

#include <AL/al.h>
#include <AL/alc.h>

#include <libs/core/core.h>

#include "al_utils.h"
#include "stb_vorbis.c"

using namespace storm;

#define CHECK_INITIALIZED()                                                                                            \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!m_isInitialized)                                                                                          \
        {                                                                                                              \
            std::stringstream ss;                                                                                      \
            ss << "[" << __FILE__ << ":" << __LINE__ << "] OGG Player is not initialized";                             \
            throw std::runtime_error(ss.str());                                                                        \
        }                                                                                                              \
    } while (false)
#define CHECK_ACTIVE_FILE_OPENED()                                                                                     \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!m_activeFile.has_value())                                                                                 \
        {                                                                                                              \
            std::stringstream ss;                                                                                      \
            ss << "[" << __FILE__ << ":" << __LINE__ << "] No file streams opened";                                    \
            throw std::runtime_error(ss.str());                                                                        \
        }                                                                                                              \
    } while (false)

namespace
{

constexpr size_t AUDIO_STREAM_SOURCE_COUNT = 2; // 2 sources: one currently playing and the second is loading
constexpr size_t AUDIO_STREAM_BUFFER_COUNT = 4;

class OggFile
{
    enum class UpdateStreamResult
    {
        Updated,
        NoData
    };

  public:
    OggFile(const std::filesystem::path &path, unsigned source,
            const std::array<unsigned, AUDIO_STREAM_BUFFER_COUNT> &buffers, size_t bufferSampleCountByChannel)
        : m_source{source}
    {
        std::ifstream file(path, std::ios::binary);
        if (!file)
        {
            throw std::runtime_error("Unable to open OGG file: " + path.string());
        }

        m_fileData = std::vector((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        int err = 0;
        m_stream = stb_vorbis_open_memory(reinterpret_cast<const unsigned char *>(m_fileData.data()),
                                          static_cast<int>(m_fileData.size()), &err, nullptr);

        const auto info = stb_vorbis_get_info(m_stream);

        m_channels = info.channels;
        m_sampleRate = info.sample_rate;
        m_format = m_channels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
        m_offset = 0;
        m_intermediateBuffer.resize(bufferSampleCountByChannel * info.channels);

        static_assert(sizeof(buffers) == sizeof(m_buffers));
        std::memcpy(m_buffers.data(), buffers.data(), m_buffers.size() * sizeof(unsigned));

        for (auto &buffer : m_buffers)
        {
            UpdateStream(buffer);
        }

        AL_CHECKED_CALL(alSourceQueueBuffers, m_source, static_cast<int>(m_buffers.size()), m_buffers.data());
    }

    ~OggFile()
    {
        stb_vorbis_close(m_stream);
    }

    void Play()
    {
        if (GetState() == AudioState::Playing)
        {
            return;
        }

        AL_CHECKED_CALL(alSourcePlay, m_source);
    }

    void Pause()
    {
        if (GetState() != AudioState::Playing)
        {
            return;
        }

        AL_CHECKED_CALL(alSourcePause, m_source);
    }

    void Stop()
    {
        if (GetState() != AudioState::Playing && GetState() != AudioState::Paused)
        {
            return;
        }

        AL_CHECKED_CALL(alSourceStop, m_source);
    }

    void SetLooped(bool isLooped)
    {
        m_isPlaybackLooped = isLooped;
    }

    AudioState GetState() const
    {
        ALint state = 0;
        AL_CHECKED_CALL(alGetSourcei, m_source, AL_SOURCE_STATE, &state);

        switch (state)
        {
        case AL_PLAYING:
            return AudioState::Playing;
        case AL_PAUSED:
            return AudioState::Paused;
        case AL_STOPPED:
            return AudioState::Stopped;
        default:
            break;
        }

        return AudioState::None;
    }

    void Update()
    {
        if (GetState() != AudioState::Playing)
        {
            return;
        }

        ALint processed = 0;
        AL_CHECKED_CALL(alGetSourcei, m_source, AL_BUFFERS_PROCESSED, &processed);
        for (ALint i = 0; i < processed; ++i)
        {
            ALuint buffer = 0;
            AL_CHECKED_CALL(alSourceUnqueueBuffers, m_source, 1, &buffer);

            auto res = UpdateStream(buffer);
            if (res == UpdateStreamResult::NoData && m_isPlaybackLooped)
            {
                stb_vorbis_seek_start(m_stream);
                m_offset = 0;
                res = UpdateStream(buffer);
            }

            if (res == UpdateStreamResult::Updated)
            {
                AL_CHECKED_CALL(alSourceQueueBuffers, m_source, 1, &buffer);
            }
        }
    }

  private:
    UpdateStreamResult UpdateStream(unsigned buffer)
    {
        auto &pcm = m_intermediateBuffer;
        std::memset(pcm.data(), 0, pcm.size() * sizeof(short));

        size_t samplesCount = 0;
        while (samplesCount < pcm.size())
        {
            const int converted = stb_vorbis_get_samples_short_interleaved(
                m_stream, m_channels, pcm.data() + samplesCount, static_cast<int>(pcm.size() - samplesCount));
            if (converted == 0)
            {
                break;
            }

            samplesCount += converted * m_channels;
        }

        if (samplesCount == 0)
        {
            return UpdateStreamResult::NoData;
        }

        m_offset += samplesCount;

        AL_CHECKED_CALL(alBufferData, buffer, m_format, pcm.data(), samplesCount * sizeof(ALshort), m_sampleRate);

        return UpdateStreamResult::Updated;
    }

  private:
    stb_vorbis *m_stream;
    std::vector<char> m_fileData;
    std::vector<short> m_intermediateBuffer; // Buffer for transferring data from decoder to OpenAL
    size_t m_offset;

    int m_format;
    int m_channels;
    int m_sampleRate;

    bool m_isPlaybackLooped;

    unsigned m_source;
    std::array<unsigned, AUDIO_STREAM_BUFFER_COUNT> m_buffers;
};

} // namespace

struct OggPlayer::Impl
{
  public:
    Impl() = default;

    ~Impl()
    {
        if (m_isInitialized)
        {
            for (auto &file : m_activeSounds)
            {
                alDeleteSources(1, &file.source); // Delete source to free buffers
                alDeleteBuffers(static_cast<int>(file.buffers.size()), file.buffers.data());
            }
        }
    }

    void Init(size_t samplesBufferSize)
    {
        if (m_isInitialized)
        {
            return;
        }

        alGetError(); // clear error code
        for (auto &file : m_activeSounds)
        {
            AL_CHECKED_CALL(alGenBuffers, static_cast<int>(file.buffers.size()), file.buffers.data());
            AL_CHECKED_CALL(alGenSources, 1, &file.source);
        }

        m_bufferSampleCountByChannel = samplesBufferSize;
        m_isInitialized = true;
    }

    void OpenFileStream(const std::filesystem::path &path)
    {
        CHECK_INITIALIZED();

        if (m_activeSounds[m_activeSound].file)
        {
            m_activeSounds[m_activeSound].file->Stop();
            AL_CHECKED_CALL(alSourcei, m_activeSounds[m_activeSound].source, AL_BUFFER, 0); // Detach all queued buffers
        }

        m_activeSound = !m_activeSound;
        m_activeSounds[m_activeSound].file =
            std::make_unique<OggFile>(path, m_activeSounds[m_activeSound].source, m_activeSounds[m_activeSound].buffers,
                                      m_bufferSampleCountByChannel);
        m_activeSounds[m_activeSound].file->SetLooped(true);
    }

    void Play()
    {
        CHECK_INITIALIZED();

        if (m_activeSounds[m_activeSound].file)
        {
            m_activeSounds[m_activeSound].file->Play();
        }
    }

    void Pause()
    {
        CHECK_INITIALIZED();

        if (m_activeSounds[m_activeSound].file)
        {
            m_activeSounds[m_activeSound].file->Pause();
        }
    }

    void Stop()
    {
        CHECK_INITIALIZED();

        if (m_activeSounds[m_activeSound].file)
        {
            m_activeSounds[m_activeSound].file->Stop();
        }
    }

    void Update()
    {
        CHECK_INITIALIZED();

        if (m_activeSounds[m_activeSound].file)
        {
            m_activeSounds[m_activeSound].file->Update();
        }
    }

    AudioState GetState() const
    {
        CHECK_INITIALIZED();

        if (m_activeSounds[m_activeSound].file)
        {
            return m_activeSounds[m_activeSound].file->GetState();
        }

        return AudioState::None;
    }

    unsigned GetSourceId() const
    {
        CHECK_INITIALIZED();

        return m_activeSounds[m_activeSound].source;
    }

  private:
    bool m_isInitialized;
    size_t m_bufferSampleCountByChannel;
    size_t m_activeSound;

    struct Sound
    {
        std::unique_ptr<OggFile> file;

        unsigned source;
        std::array<unsigned, AUDIO_STREAM_BUFFER_COUNT> buffers;
    };

    std::array<Sound, AUDIO_STREAM_SOURCE_COUNT> m_activeSounds;
};

OggPlayer::OggPlayer() : m_impl(std::make_unique<Impl>())
{
}

OggPlayer::~OggPlayer() = default;

bool OggPlayer::Init(size_t samplesBufferSize /*= 1024*/)
try
{
    m_impl->Init(samplesBufferSize);
    return true;
}
catch (const std::runtime_error &e)
{
    core.Trace("%s\n", e.what());
    return false;
}

bool OggPlayer::OpenFileStream(const std::filesystem::path &path)
try
{
    m_impl->OpenFileStream(path);
    return true;
}
catch (const std::runtime_error &e)
{
    core.Trace("%s\n", e.what());
    return false;
}

void OggPlayer::Play()
try
{
    m_impl->Play();
}
catch (const std::runtime_error &e)
{
    core.Trace("%s\n", e.what());
}

void OggPlayer::Pause()
try
{
    m_impl->Pause();
}
catch (const std::runtime_error &e)
{
    core.Trace("%s\n", e.what());
}

void OggPlayer::Stop()
try
{
    m_impl->Stop();
}
catch (const std::runtime_error &e)
{
    core.Trace("%s\n", e.what());
}

void OggPlayer::Update()
try
{
    m_impl->Update();
}
catch (const std::runtime_error &e)
{
    core.Trace("%s\n", e.what());
}

AudioState OggPlayer::GetState() const
try
{
    return m_impl->GetState();
}
catch (const std::runtime_error &e)
{
    core.Trace("%s\n", e.what());
    return AudioState::None;
}

unsigned OggPlayer::GetSourceId() const
try
{
    return m_impl->GetSourceId();
}
catch (const std::runtime_error &e)
{
    core.Trace("%s\n", e.what());
    return 0;
}
