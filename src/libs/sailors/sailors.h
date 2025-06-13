#pragma once

#include <libs/collide/collide.h>
#include <libs/model/model.h>
#include <libs/renderer/dx9render.h>
#include <libs/sea/sea_base.h>
#include <libs/shared_headers/sea_ai/sea_people.h>
#include <libs/ship/ship_base.h>

#include "sailors_way_points.h"

enum ManMode {
    MAN_JUMP,
    MAN_SWIM,
    MAN_OFF,
    MAN_WALK,
    MAN_RUN,
    MAN_STAY,
    MAN_CANNONRELOAD,
    MAN_TURNLEFT,
    MAN_TURNRIGHT,
    MAN_CLIMB_UP,
    MAN_CLIMB_DOWN
};

enum ManMoveTo { MOVE_TO_POINT, MOVE_TO_CANNON, MOVE_TO_TOP };

struct ShipState {
    uint32_t  mode;
    bool      dead;
    SEA_BASE* sea;

    ShipState() : sea(nullptr)
    {
        mode = SHIP_SAIL;
        dead = false;
    }
};

class ShipMan
{
public:
    ShipMan(ShipMan const& other)            = delete;
    ShipMan& operator=(ShipMan const& other) = delete;

    ShipMan();
    ~ShipMan();

    ShipMan(ShipMan&& other) noexcept;
    ShipMan& operator=(ShipMan&& other) noexcept;

    // Building a matrix taking into account the current state
    void SetPos(MODEL* ship, SHIP_BASE* ship_base, uint32_t& dltTime, ShipState& shipState);
    void FindNextPoint(SailorsPoints& sailorsPoints, ShipState& shipState);
    int  FindRandomPoint(SailorsPoints& sailorsPoints, ShipState& shipState);
    int  FindRandomPointWithoutType(SailorsPoints const& sailorsPoints) const;
    void ApplyTargetPoint(CVECTOR pt, bool randomWalk);

    void UpdatePos(uint32_t& dltTime, SailorsPoints& sailorsPoints, ShipState& shipState);
    // Update animation and speed
    void SetAnimation(uint32_t dltTime, ShipState& shipState);

    bool MoveToPosition(uint32_t& dltTime, SailorsPoints& sailorsPoints, ShipState& shipState);
    bool RotateToAngle(uint32_t& dltTime, SailorsPoints& sailorsPoints);
    bool Stay(uint32_t& dltTime, SailorsPoints& sailorsPoints) const;
    bool Turn(uint32_t& dltTime, SailorsPoints& sailorsPoints);
    bool Swim(uint32_t& dltTime, SailorsPoints& sailorsPoints, ShipState& shipState);
    bool Jump(uint32_t& dltTime, SailorsPoints& sailorsPoints, ShipState& shipState);

    void NewAction(SailorsPoints& sailorsPoints, ShipState& shipState, uint32_t& dltTime);
    int  GetNearestEmptyCannon(SailorsPoints& sailorsPoints) const;

    entid_t modelID {};

    CVECTOR pos {}, ang {};  // current position
    CVECTOR ptTo {}, angTo {}, dir {};

    CVECTOR spos {};  // Bypassing each other
    // float sang; // Bypass each other

    float dieTime {};
    bool  inWater {};
    float jumpSpeedX {}, jumpSpeedY {};

    Path path;  // Current path

    ManMode mode, lastMode;  // Mode
    int     newWayPoint {}, lastWayPoint {}, targetWayPoint {}, lastTargetPoint {};

    ManMoveTo moveTo {};  // Current point type

    float manSpeed;
    float rotSpeed;
};

class ShipWalk
{
public:
    void ReloadCannons(int bort);

    void CreateNewMan(SailorsPoints& sailorsPoints);

    bool Init(entid_t _shipID, int editorMode, char const* shipType, std::vector<std::string>&& shipManModels);
    void CheckPosition(uint32_t const& dltTime);
    void SetMastBroken(int iMastIndex);
    void OnHullHit(const CVECTOR& v);

    SHIP_BASE* ship;
    MODEL*     shipModel;

    bool bHide;

    entid_t shipID;

    SailorsPoints sailorsPoints;  // Points
    ShipState     shipState;      // Ship state

    std::vector<ShipMan>     shipMan;
    std::vector<std::string> shipManModels_ = {
        "lowcharacters/lo_man_1",
        "lowcharacters/lo_man_2",
        "lowcharacters/lo_man_3",
        "lowcharacters/lo_man_kamzol_1",
        "lowcharacters/lo_man_kamzol_2",
        "lowcharacters/lo_man_kamzol_3"};
};

class Sailors: public Entity
{
public:
    Sailors();

    bool         Init(std::shared_ptr<storm::ServiceLocator> const& service_locator) override;
    virtual void Realize(uint32_t dltTime);

    uint64_t ProcessMessage(MESSAGE& message) override;
    uint32_t AttributeChanged(ATTRIBUTES* attr) override;

    void ProcessStage(Stage stage, uint32_t delta) override
    {
        switch (stage) {
        case Stage::realize: Realize(delta); break;
        }
    }

    VDX9RENDER*           rs;
    std::vector<ShipWalk> shipWalk;
    bool                  editorMode;
    bool                  disabled;
};
