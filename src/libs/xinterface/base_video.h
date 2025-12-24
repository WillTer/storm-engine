#pragma once

#include <libs/core/entity.h>

class xiBaseVideo: public Entity
{
public:
    virtual void SetShowVideo(bool bShowVideo) {}

    virtual /*IDirect3DTexture9*/ void* GetCurrentVideoTexture()
    {
        return nullptr;
    }
};
