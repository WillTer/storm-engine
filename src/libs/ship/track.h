#pragma once

#include <vector>

#include <libs/sea/sea_base.h>

#include "ship_base.h"

class ShipTracks: public Entity
{
public:
    ShipTracks() = default;
    ~ShipTracks() override;

    bool Init() override;

    void Execute(uint32_t dwDeltaTime);
    void Realize(uint32_t dwDeltaTime);

    void ProcessStage(Stage stage, uint32_t delta) override
    {
        switch (stage) {
        case Stage::execute: Execute(delta); break;
        case Stage::realize: Realize(delta); break;
        }
    }

    void AddShip(SHIP_BASE* pShip);
    void DelShip(SHIP_BASE* pShip);
    void ResetTrack(SHIP_BASE* pShip);

    uint32_t AttributeChanged(ATTRIBUTES* pA) override;

private:
    class ShipTrack
    {
    public:
        static SEA_BASE* pSea;
        static int32_t   iVTmpBuffer1, iVTmpBuffer2;
        static int32_t   iITmpBuffer1, iITmpBuffer2;
        static int32_t   iRefCount;
        static uint32_t  dwMaxBufferSize1, dwMaxBufferSize2;

        SHIP_BASE* pShip;

        ShipTrack();
        ~ShipTrack();

        bool Update(SHIP_BASE* pShip);
        void Reset();

        void Execute(float fDeltaTime);
        void Realize(float fDeltaTime);

    private:
        struct Track {
            CVECTOR vPos;
            float   fCos, fSin;
            float   fTime;
            float   fTV;
            float   fAlpha, fInitialAlpha;
            float   fWidth;
            float   fSpeed;
        };

        struct TrackVertex {
            CVECTOR  vPos;
            uint32_t dwColor;
            float    tu, tv;
        };

        bool bFirstExecute;

        float   fCurTV;
        CVECTOR vLastPos, vLastAng;

        std::vector<Track> aTrack1, aTrack2;
        int32_t            iTrackTexture1, iTrackTexture2;
        float              fWidth11, fWidth12, fWidth21, fWidth22;
        float              fSpeed11, fSpeed12, fSpeed21, fSpeed22;
        float              fLifeTime1, fLifeTime2;
        float              fZStart1, fZStart2;
        float              fTrackStep1, fTrackStep2;
        uint32_t           dwTrackStep1, dwTrackStep2;
        float              fUP1, fUP2;

        float fTrackDistance;

        bool Reserve1(uint32_t dwSize);
        bool Reserve2(uint32_t dwSize);
    };

    std::vector<ShipTrack*> aShips;
};
