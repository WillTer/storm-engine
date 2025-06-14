#pragma once

#include <vector>

#include <libs/collide/collide.h>
#include <libs/core/save_load.h>
#include <libs/island/island_base.h>
#include <libs/location/character_helpers.h>
#include <libs/renderer/dx9render.h>

class AIAttributesHolder
{
protected:
    ATTRIBUTES* pACharacter;

public:
    virtual void SetACharacter(ATTRIBUTES* pAP)
    {
        pACharacter = pAP;
    };

    virtual ATTRIBUTES* GetACharacter() const
    {
        return pACharacter;
    };
};

class VAI_INNEROBJ;

class AIHelper
{
public:
    AIHelper();
    ~AIHelper();

    static ATTRIBUTES*  pASeaCameras;
    static ISLAND_BASE* pIsland;
    static VDX9RENDER*  pRS;

    static float fGravity;

    bool SetDevice();
    bool Init(std::shared_ptr<entt::registry> const& registry);
    bool Uninit();
    void AddCharacter(ATTRIBUTES* pACharacter, ATTRIBUTES* pAMainCharacter);
    void CalculateRelations();

    bool isFriend(ATTRIBUTES* pA1, ATTRIBUTES* pA2) const;
    bool isEnemy(ATTRIBUTES* pA1, ATTRIBUTES* pA2) const;
    bool isNeutral(ATTRIBUTES* pA1, ATTRIBUTES* pA2) const;

    ATTRIBUTES* GetMainCharacter(ATTRIBUTES* pACharacter);

    static VAI_INNEROBJ* FindAIInnerObj(ATTRIBUTES* pACharacter);

    uint32_t GetRelation(ATTRIBUTES* pA1, ATTRIBUTES* pA2) const;
    uint32_t GetRelationSafe(ATTRIBUTES* pA1, ATTRIBUTES* pA2) const;

    static void Print(float x, float y, float fScale, char const* pFormat, ...);
    static void Print3D(CVECTOR vPos, float dy, float fScale, char const* pFormat, ...);

    void Save(CSaveLoad* pSL);
    void Load(CSaveLoad* pSL);

private:
    std::shared_ptr<entt::registry> m_registry;

    uint32_t *               pRelations, dwRelationSize;
    std::vector<ATTRIBUTES*> aCharacters, aMainCharacters;

    uint32_t* GetRelation(uint32_t x, uint32_t y) const;
    uint32_t  FindIndex(ATTRIBUTES* pACharacter) const;
};

extern AIHelper Helper;
