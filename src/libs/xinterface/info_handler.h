#pragma once

#include <libs/core/entity.h>

class InfoHandler: public Entity
{
public:
    InfoHandler();
    ~InfoHandler() override;
    bool     Init() override;
    void     Execute(uint32_t delta_time);
    void     Realize(uint32_t delta_time) const;
    uint64_t ProcessMessage(MESSAGE& message) override;

    void ProcessStage(Stage stage, uint32_t delta) override
    {
        switch (stage) {
        case Stage::execute: Execute(delta); break;
        case Stage::realize: Realize(delta); break;
        }
    }

protected:
    void        StringToBufer(char* outStr, int sizeBuf, char const* inStr, int copySize) const;
    char const* GetCutString(char const* pstr, int nOutWidth, float fScale) const;
    bool        DoPreOut();

    struct {
        float x, y, z, rhw;
        float u, v;
    } drawbuf_base[6];
};
