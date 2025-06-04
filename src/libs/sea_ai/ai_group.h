#pragma once

#include <libs/core/vma.hpp>

#include "ai_ship.h"

// ============================================================================
// Master class AIGroup
// Contain base virtual functions
// ============================================================================
class AIGroup
{
private:
    DTimer      dtCheckTask;
    std::string sCommand, sCommandGroup;
    std::string sGroupName;
    std::string sLocationNearOtherGroup;
    std::string sGroupType;
    CVECTOR     vInitGroupPos {};
    CVECTOR     vMovePoint {};
    bool        bFirstExecute;

    ATTRIBUTES* pACommander;

    // ship container for this group
    std::vector<AIShip*> aGroupShips;

public:
    AIGroup() {};
    AIGroup(char const* pGroupName);
    virtual ~AIGroup();

    // global group container, accessible for AIGroup and SEA_AI.
    static std::vector<AIGroup*> AIGroups;
    static float                 fDistanceBetweenGroupShips;
    static float                 fDistanceBetweenGroupLines;

    // static functions
    static AIGroup* FindGroup(ATTRIBUTES* pACharacter);
    static AIGroup* FindGroup(char const* pGroupName);
    static AIGroup* FindOrCreateGroup(char const* pGroupName);
    static AIGroup* CreateNewGroup(char const* pGroupName);
    static AIGroup* FindMainGroup();
    static float    GetAttackHP(char const* pGroupName, float fDistance);
    static void     SetXYZ_AY(char const* pGroupName, CVECTOR vPos, float _fAY);
    static void     GroupSetMove(char const* pGroupName, CVECTOR& vMovePoint);
    static void     GroupSetAttack(char const* cGroupName, char const* cGroupAttackingName);
    static void     GroupSetAttack(AIShip* pS1, AIShip* pS2);
    static void     GroupSetRunAway(char const* pGroupName);
    static void     GroupSetType(char const* pGroupName, char const* cGroupType);
    static void     GroupSetCommander(char const* pGroupName, ATTRIBUTES* _pACommander);
    static void     GroupSetLocationNearOtherGroup(char const* pGroupName, char const* pOtherGroupName);
    static void     ShipChangeGroup(ATTRIBUTES* pACharacter, char const* pGroupName);
    static void     SwapCharactersShips(ATTRIBUTES* pACharacter1, ATTRIBUTES* pACharacter2);
    static void     SetOfficerCharacter2Ship(ATTRIBUTES* pOfficerCharacter, ATTRIBUTES* pReplacedACharacter);
    static void     SailMainGroup(CVECTOR vPos, float fAngleY, ATTRIBUTES* pACharacter);

    static void GroupHelpMe(char const* pGroupName, AIShip* pMe, AIShip* pEnemy);

    // AI section
    bool  isAttack(AIGroup* pGroup);
    bool  isDead();  // is group dead?
    bool  isMainGroup();
    float GetPower();

    // return group name
    std::string& GetName()
    {
        return sGroupName;
    };

    // extract / insert ship group
    AIShip* ExtractShip(ATTRIBUTES* pACharacter);
    void    InsertShip(AIShip* pAIShip);

    // add new ship to group
    ATTRIBUTES* GetCommanderACharacter() const;
    AIShip*     GetMainShip();
    void        AddShip(entid_t _eidShip, ATTRIBUTES* pACharacter, ATTRIBUTES* pAShip);

    void Unload() {};

    // execute/realize function (on each frame)
    void Realize(float fDeltaTime);
    void Execute(float fDeltaTime);

    void Save(CSaveLoad* pSL);
    void Load(CSaveLoad* pSL);
};
