#pragma once

#include <stdexcept>

#include "v_file_service.h"

#define KNF_KEY 0x1
#define KNF_COMMENTARY 0x2

class FileService;

class KEY_NODE
{
    char* key_name;
    char* key_val;

    int32_t name_size;
    int32_t val_size;

    KEY_NODE* l_PTR;
    KEY_NODE* r_PTR;

    uint32_t flags;

public:
    KEY_NODE();
    ~KEY_NODE();
    void      SetName(char const* name);
    void      SetValue(char const* value);
    char*     GetName();
    char*     GetValue();
    KEY_NODE* GetLeftNode();
    KEY_NODE* GetRightNode();
    void      SetLeftNode(KEY_NODE* node);
    void      SetRightNode(KEY_NODE* node);
    void      Add(KEY_NODE** Root, KEY_NODE** Top);
    void      AttachTo(KEY_NODE* node, KEY_NODE** Top);  // left - root, order - from left to right
    void      Deattach(KEY_NODE** Root, KEY_NODE** Top);
    uint32_t  SetFlags(uint32_t flags);
};

class SECTION
{
    KEY_NODE* Root;
    KEY_NODE* Top;

    SECTION* l_PTR;
    SECTION* r_PTR;

    char* Name;

public:
    SECTION();
    ~SECTION();

    KEY_NODE* AddNode();
    void      DelNode(KEY_NODE*);

    KEY_NODE* FindKey(KEY_NODE* from, char const* key_name, char const* key_value);
    KEY_NODE* FindKey(char const* key_name, char const* key_value);
    KEY_NODE* FindKey(char const* key_name);
    KEY_NODE* GetRoot();

    void  SetName(char const* name);
    char* GetName();

    SECTION* GetLeftNode();
    SECTION* GetRightNode();
    void     SetLeftNode(SECTION* node);
    void     SetRightNode(SECTION* node);
    void     Add(SECTION** SRoot, SECTION** STop);
    void     Deattach(SECTION** SRoot, SECTION** STop);
};

typedef struct {
    SECTION*  Section;
    KEY_NODE* Key;
} SEARCH_DATA;

class IFS: public INIFILE
{
    friend FileService;

protected:
    SECTION* SectionSNode;

    SECTION* SectionRoot;
    SECTION* SectionTop;

    // KEY_NODE * FindKey(KEY_NODE * from, char * section_name, char * key_name, char * key_value);
    KEY_NODE* FindKey(char const* section_name, char const* key_name, char const* key_value);
    KEY_NODE* FindKey(char const* section_name, char const* key_name);
    SECTION*  FindSection(char const* section_name);
    SECTION*  FindSection(char const* section_name, SECTION* snode);

    void Format(char* file_data, int32_t file_size);
    bool VoidSym(char symbol);

    IFileService*         fs;
    std::filesystem::path FileName;
    bool                  bDataChanged;
    uint32_t              Reference;
    uint32_t              CompareStrings(char const* s1, char const* s2);
    bool                  FlushFile();

public:
#define IFS_NOT_IMPLEMENTED(x) \
    x override \
    { \
        throw std::logic_error("not implemented"); \
    }
    IFS_NOT_IMPLEMENTED(void ReadString(const char* section_name, const char* key_name, char* buffer, size_t buffer_size))
    IFS_NOT_IMPLEMENTED(
        bool ReadString(char const* section_name, char const* key_name, char* buffer, size_t buffer_size, char const* def_string))
    IFS_NOT_IMPLEMENTED(bool ReadStringNext(char const* section_name, char const* key_name, char* buffer, size_t buffer_size))
    IFS_NOT_IMPLEMENTED(int32_t GetInt(char const* section_name, char const* key_name))
    IFS_NOT_IMPLEMENTED(int32_t GetInt(char const* section_name, char const* key_name, int32_t def_val))
    IFS_NOT_IMPLEMENTED(bool GetIntNext(char const* section_name, char const* key_name, int32_t* val))
    IFS_NOT_IMPLEMENTED(double GetDouble(char const* section_name, char const* key_name))
    IFS_NOT_IMPLEMENTED(double GetDouble(char const* section_name, char const* key_name, double def_val))
    IFS_NOT_IMPLEMENTED(bool GetDoubleNext(char const* section_name, char const* key_name, double* val))
    IFS_NOT_IMPLEMENTED(float GetFloat(char const* section_name, char const* key_name))
    IFS_NOT_IMPLEMENTED(float GetFloat(char const* section_name, char const* key_name, float def_val))
    IFS_NOT_IMPLEMENTED(bool GetFloatNext(char const* section_name, char const* key_name, float* val))

    bool CaseSensitive(bool /*yes*/) override
    {
        return false;
    }
#undef IFS_NOT_IMPLEMENTED

    IFS(IFileService* _fs);
    ~IFS();

    bool LoadFile(std::filesystem::path const& file_path);

    std::filesystem::path const& GetFileName()
    {
        return FileName;
    };
    void    IncReference();
    void    DecReference();
    int32_t GetReference();

    SECTION* CreateSection(char const* section_name);
    void     DeleteSection(char const* section_name) override;

    bool TestKey(char const* section_name, char const* key_name, char const* key_value) override;
    void DeleteKey(char const* section_name, char const* key_name) override;
    void DeleteKey(char const* section_name, char const* key_name, char const* key_value) override;

    bool
    ReadString(SEARCH_DATA* sd, char const* section_name, char const* key_name, char* buffer, uint32_t buffer_size, char const* def_string);
    void ReadString(
        SEARCH_DATA* sd,
        char const*  section_name,
        char const*  key_name,
        char*        buffer,
        uint32_t     buffer_size);  // rise exception on fault
    bool ReadStringNext(SEARCH_DATA* sd, char const* section_name, char const* key_name, char* buffer, uint32_t buffer_size);

    int32_t GetInt(SEARCH_DATA* sd, char const* section_name, char const* key_name);
    int32_t GetInt(SEARCH_DATA* sd, char const* section_name, char const* key_name, int32_t def_val);
    bool    GetIntNext(SEARCH_DATA* sd, char const* section_name, char const* key_name, int32_t* val);

    double GetDouble(SEARCH_DATA* sd, char const* section_name, char const* key_name);
    double GetDouble(SEARCH_DATA* sd, char const* section_name, char const* key_name, double def_val);
    bool   GetDoubleNext(SEARCH_DATA* sd, char const* section_name, char const* key_name, double* val);

    float GetFloat(SEARCH_DATA* sd, char const* section_name, char const* key_name);
    float GetFloat(SEARCH_DATA* sd, char const* section_name, char const* key_name, float def_val);
    bool  GetFloatNext(SEARCH_DATA* sd, char const* section_name, char const* key_name, float* val);

    void AddString(char const* section_name, char const* key_name, char const* string) override;
    void WriteString(char const* section_name, char const* key_name, char const* string) override;
    void WriteLong(char const* section_name, char const* key_name, int32_t value) override;
    void WriteDouble(char const* section_name, char const* key_name, double value) override;
    void WriteFloat(char const* section_name, char const* key_name, float value);

    bool GetSectionName(char* section_name_buffer, int32_t buffer_size) override;
    bool GetSectionNameNext(char* section_name_buffer, int32_t buffer_size) override;
    void Flush() override;
    bool Reload() override;
    bool TestSection(char const* section_name) override;
};
