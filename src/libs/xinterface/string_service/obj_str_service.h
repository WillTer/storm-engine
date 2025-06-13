#pragma once

#include <libs/core/entity.h>

#include "../string_service.h"

class OBJ_STRSERVICE: public Entity
{
    VSTRSERVICE* m_pStrService;

public:
    OBJ_STRSERVICE();
    ~OBJ_STRSERVICE() override;
    bool     Init(std::shared_ptr<storm::ServiceLocator> const& service_locator) override;
    uint64_t ProcessMessage(MESSAGE& message) override;

    void ProcessStage(Stage, uint32_t) override {}
};
