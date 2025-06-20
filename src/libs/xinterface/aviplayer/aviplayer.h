#pragma once

#include <libs/math/c_vector.h>
#include <libs/renderer_next/types.h>

#include "../base_video.h"

// #define XI_AVIVIDEO_FVF (D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_TEXTUREFORMAT2)

struct XI_AVIVIDEO_VERTEX {
    CVECTOR pos;
    float   w;
    float   tu, tv;
};

class CAviPlayer: public xiBaseVideo
{
    bool m_bShowVideo;

public:
    CAviPlayer();
    ~CAviPlayer() override;
    bool     Init() override;
    void     Execute(uint32_t delta_time);
    void     Realize(uint32_t delta_time);
    uint64_t ProcessMessage(MESSAGE& message) override;

    void ProcessStage(Stage stage, uint32_t delta) override
    {
        switch (stage) {
        case Stage::execute: Execute(delta); break;
        case Stage::realize: Realize(delta); break;
        }
    }

    void SetShowVideo(bool bShowVideo) override
    {
        m_bShowVideo = bShowVideo;
    }

    /*IDirect3DTexture9*/ void* GetCurrentVideoTexture() override
    {
        return nullptr;
    }

protected:
    bool m_bContinue;

    storm::Point dstPnt;
    storm::Rect  lockRect;

    XI_AVIVIDEO_VERTEX v[4];

    void ReleaseAll();
    bool PlayMedia(char const* fileName);
    bool GetInterfaces();
    void CleanupInterfaces();

    bool m_bFirstDraw;
    bool m_bMakeUninitializeDD;

    std::string filename;
    bool        bLoop {false};
};
