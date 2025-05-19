#include "al_backend.h"

#include "al_channel.h"
#include "al_sound.h"

using namespace storm::audio;

ALBackend::ALBackend()  = default;
ALBackend::~ALBackend() = default;

Result ALBackend::init()
{
    return Result::Ok;
}

Result ALBackend::create_sound(std::filesystem::path const& file, std::shared_ptr<ISound>& out)
{
    out = std::make_shared<ALSound>();
    return Result::Ok;
}

Result ALBackend::create_sound_stream(std::filesystem::path const& file, std::shared_ptr<ISound>& out)
{
    out = std::make_shared<ALSound>();
    return Result::Ok;
}

Result ALBackend::bind_sound(std::shared_ptr<ISound> const& sound, std::shared_ptr<IChannel>& out)
{
    out = std::make_shared<ALChannel>();
    return Result::Ok;
}

Result ALBackend::set_listener_position_3d(std::array<float, 3> const& position)
{
    return Result::Ok;
}

Result ALBackend::set_listener_velocity_3d(std::array<float, 3> const& velocity)
{
    return Result::Ok;
}

Result ALBackend::set_listener_orientation_3d(std::array<float, 3> const& orientation)
{
    return Result::Ok;
}

void ALBackend::update() {}
