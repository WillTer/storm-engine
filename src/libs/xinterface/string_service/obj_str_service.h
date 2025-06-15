#pragma once

#include <libs/core/entity.h>

#include "../string_service.h"

class ObjStrService: public Entity
{
    VSTRSERVICE* m_pStrService;

public:
    ObjStrService();
    ~ObjStrService() override;
    bool     Init() override;
    uint64_t ProcessMessage(MESSAGE& message) override;

    void ProcessStage(Stage, uint32_t) override {}
};
