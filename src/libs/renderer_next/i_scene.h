#pragma once

#include <cstdint>

namespace storm
{

class IScene
{
public:
    virtual ~IScene() = default;

    virtual void update(uint64_t delta_time) = 0;
    virtual void render() const              = 0;
};

}  // namespace storm
