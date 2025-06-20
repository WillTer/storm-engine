#pragma once

#include <libs/core/entity.h>

class ScrShoter: public Entity
{
    struct SAVETEXTURES {
        char*         fileName;
        char*         dataString;
        int32_t       textureId = -1;
        SAVETEXTURES* next;
    };

public:
    ~ScrShoter() override;
    void     SetDevice();
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

private:
    bool    MakeScreenShot();
    int32_t FindSaveTexture(char const* fileName) const;
    char*   FindSaveData(char const* fileName) const;
    int32_t AddSaveTexture(char const* dirName, char const* fileName);
    void    DelSaveTexture(char const* fileName);
    int32_t GetTexFromSave(char* fileName, char** pDatStr) const;

    int32_t       textureIndex_ = -1;
    SAVETEXTURES* m_list        = nullptr;
};
