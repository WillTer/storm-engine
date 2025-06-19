#pragma once

#define MAX_CANNON_NAME_LENGTH 100

struct ship_t {
    int32_t iClass;
    float   fSpeedRate, fTurnRate;
    int32_t iCrewMin, iCrewMax;
    int32_t iCapacity;
    float   fLength, fWidth;
    float   fWeight;
    char    name[MAX_CANNON_NAME_LENGTH];
    float   fSpeedDependW, fSubSeaDependW, fTurnDependW, fTurnDependS;
    float   fWaterLine;
};

/// for avigrabber
#define MSG_SHIP_SET_POS 5002
