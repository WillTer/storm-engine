#pragma once

#include <string>

#include <libs/core/service.h>

namespace storm
{
class Font;
}

class VSTRSERVICE: public SERVICE
{
public:
    bool Init() override                                   = 0;
    bool LoadState(ENTITY_STATE* state) override           = 0;
    bool CreateState(ENTITY_STATE_GEN* state_gen) override = 0;
    void RunStart() override                               = 0;
    void RunEnd() override                                 = 0;

    virtual void  SetLanguage(char const* sLanguage) = 0;
    virtual char* GetLanguage()                      = 0;

    virtual char*   GetString(char const* stringName, char* sBuffer = nullptr, std::size_t bufferSize = 0) = 0;
    virtual int32_t GetStringNum(char const* stringName)                                                   = 0;
    virtual char*   GetString(int32_t strNum)                                                              = 0;
    virtual char*   GetStringName(int32_t strNum)                                                          = 0;

    virtual int32_t OpenUsersStringFile(char const* fileName)         = 0;
    virtual void    CloseUsersStringFile(int32_t id)                  = 0;
    virtual char*   TranslateFromUsers(int32_t id, char const* inStr) = 0;

    virtual auto get_font(std::string const& name) -> std::shared_ptr<storm::Font> = 0;
};
