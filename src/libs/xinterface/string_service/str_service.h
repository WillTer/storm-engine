#pragma once

#include <libs/core/script_libriary.h>
#include <libs/core/vma.hpp>

#include "../string_service.h"

//-----------SDEVICE-----------
class STRSERVICE: public VSTRSERVICE
{
    struct UsersStringBlock {
        int32_t nref;
        char*   fileName;
        int32_t blockID;
        int32_t nStringsQuantity;
        char**  psStrName;
        char**  psString;

        UsersStringBlock* next;
    };

public:
    STRSERVICE();
    ~STRSERVICE() override;

    bool Init(std::shared_ptr<entt::registry> const& registry) override;
    bool LoadState(ENTITY_STATE* state) override;
    bool CreateState(ENTITY_STATE_GEN* state_gen) override;
    void RunStart() override;
    void RunEnd() override;

    void  SetLanguage(char const* sLanguage) override;
    char* GetLanguage() override;

    char*   GetString(char const* stringName, char* sBuffer = nullptr, std::size_t bufferSize = 0) override;
    int32_t GetStringNum(char const* stringName) override;
    char*   GetString(int32_t strNum) override;
    char*   GetStringName(int32_t strNum) override;

    int32_t OpenUsersStringFile(char const* fileName) override;
    void    CloseUsersStringFile(int32_t id) override;
    char*   TranslateFromUsers(int32_t id, char const* inStr) override;

    void SetDialogSourceFile(char const* fileName);

    char* TranslateForDialog(char const* str)
    {
        return TranslateFromUsers(m_nDialogSourceFile, (char*)str);
    }

protected:
    void    LoadIni();
    int32_t GetFreeUsersID() const;
    bool    GetNextUsersString(char* src, int32_t& idx, char** strName, char** strData) const;

protected:
    char* m_sLanguage;
    char* m_sIniFileName;
    char* m_sLanguageDir;

    int32_t m_nStringQuantity;
    char**  m_psStrName;
    char**  m_psString;

    UsersStringBlock* m_pUsersBlocks;

    int32_t m_nDialogSourceFile;
};

class SCRIPT_INTERFACE_FUNCTIONS: public SCRIPT_LIBRIARY
{
public:
    SCRIPT_INTERFACE_FUNCTIONS() {};

    ~SCRIPT_INTERFACE_FUNCTIONS() override {};
    bool Init() override;
};
