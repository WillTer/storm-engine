#pragma once

#include <libs/core/core.h>
#include <libs/core/entity.h>
#include <libs/math/matrix.h>

#include "../xdefines.h"

class MODEL;
class NODE;
class CMatrix;

class InterfaceBackScene: public Entity
{
    struct LightParam {
        bool bUse;

        int32_t indexLight;

        uint32_t dwFlareColor;
        float    fMinFlareColor;
        float    fMaxFlareColor;

        float fColorTimer;
        float fColorPeriod;
        float fAddPeriod;
        float fAddPeriodMax;

        float fRangeMin, fRangeMax;
        float fRangeTimer;
        float fRangePeriod;

        MODEL*  pModel;
        entid_t eiModel;

        CVECTOR vLightPos;
        NODE*   pLightSrcNode;

        LightParam()
        {
            bUse          = false;
            pModel        = nullptr;
            pLightSrcNode = nullptr;
            fColorTimer   = 0.0f;
        }

        ~LightParam();
        void UpdateParams(float fTime);
    };

public:
    InterfaceBackScene();
    ~InterfaceBackScene() override;

    bool     Init() override;
    void     Execute(uint32_t Delta_Time);
    void     Realize(uint32_t Delta_Time);
    uint64_t ProcessMessage(MESSAGE& message) override;

    void ProcessStage(Stage stage, uint32_t delta) override
    {
        switch (stage) {
        case Stage::execute: Execute(delta); break;
        case Stage::realize: Realize(delta); break;
        }
    }

protected:
    entid_t m_eiModel;
    MODEL*  m_pModel;
    entid_t m_eiLocators;
    MODEL*  m_pLocators;

    FXYPOINT m_pntOldMouse;
    CVECTOR  m_vCamPos;
    CVECTOR  m_vCamAng;
    float    m_fCamPerspective;

    std::vector<LightParam*> m_aLights;

    struct MenuDescr {
        bool        bSelectable;
        entid_t     eiActive;
        MODEL*      pActive;
        entid_t     eiPassive;
        MODEL*      pPassive;
        std::string sEventName;

        MenuDescr() : eiActive(0), eiPassive(0)
        {
            pActive     = nullptr;
            pPassive    = nullptr;
            bSelectable = false;
        }

        ~MenuDescr();
        void
        Set(CMatrix*    pMtx,
            char const* pcActiveName,
            char const* pcPassiveName,
            char const* pcEvent,
            char const* pcPathName,
            char const* pcTechniqueName);
    };

    std::vector<MenuDescr*> m_aMenuDescr;
    int32_t                 m_nSelectMenuIndex;

    void LoadModel(char const* pcModelName);
    void SetCameraPosition(char const* pcLocatorName);
    void SetShipPosition(char const* pcLocName, ATTRIBUTES* pAChar) const;

    bool FindLocator(char const* pcLocName, CMatrix* pMtx, CVECTOR* pPos, float* pYAng) const;
    void SetLocatorPosition(MODEL* pModel, char const* pcLocName, CVECTOR& pos, NODE*& pNodPtr);

    void ReleaseMenuList();
    void CreateMenuList(int32_t nStartIndex, ATTRIBUTES* pAMenu);
    void ChooseNextMenu();
    void ChoosePrevMenu();
    void SetNewMenu(int32_t nNewSelectIndex);
    void SetMenuSelectableState(int32_t nMenuIndex, bool bSelectable);
    void ExecuteMenu(int32_t nMenuIndex);

    int32_t CheckMousePos(float fX, float fY);

    void InitLight(ATTRIBUTES* pAParam);
    void SetLight();
    void RestoreLight();
    void FlareShow(int32_t idx);

    void InitAniModel(ATTRIBUTES* pAParam);
    void InitStaticModel(ATTRIBUTES* pAParam);

    struct AniModelDescr {
        entid_t ei;
        MODEL*  pModel;

        bool     bUseTFactor;
        uint32_t dwTFactor;

        AniModelDescr() : ei(0), dwTFactor(0)
        {
            pModel      = nullptr;
            bUseTFactor = false;
        }

        ~AniModelDescr()
        {
            core->EraseEntity(ei);
            pModel = nullptr;
        }
    };

    std::vector<AniModelDescr*> m_apAniModel;

    // a fly is a migratory bird!
    struct Particle {
        CVECTOR pos;
        float   angle;
        float   size;
        float   alpha;
    };

    struct ParticleEx: public Particle {
        uint32_t color;
        float    frame;
    };

    struct ParticleFly: public ParticleEx {
        float ax, ay;
        float kx, ky;
        float a, k;
    };

    struct LampFlys {
        CVECTOR pos;
        float   radius;
        int32_t start;
        int32_t num;
    };

    struct Vertex {
        CVECTOR  pos;
        uint32_t color;
        float    u, v;
    };

    std::vector<LampFlys>    flys;
    int32_t                  numFlys;
    int32_t                  maxFlys;
    std::vector<ParticleFly> fly;
    int32_t                  numFly;
    int32_t                  flyTex;
    Vertex                   buffer[256 * 6];

    CVECTOR m_vFlarePos;
    float   m_fFlareSize;
    int32_t m_nFlareTexture;

    void AddLampFlys(CVECTOR& pos);
    void ProcessedFlys(float dltTime);
    void DrawParticles(void* prts, int32_t num, int32_t size, int32_t texture, char const* tech, bool isEx = false, int32_t numU = 0);
};
