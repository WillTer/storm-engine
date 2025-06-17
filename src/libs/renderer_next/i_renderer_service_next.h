#pragma once

namespace storm
{

class IRendererServiceNext
{
public:
    virtual ~IRendererServiceNext() = default;

    virtual bool Init() = 0;
};

}  // namespace storm