#pragma once

#include <memory>

#include <libs/config/main_config.h>

#include "ifs.h"
#include "v_file_service.h"

#define _MAX_OPEN_INI_FILES 1024

class INIFILE_T: public INIFILE
{
public:
    INIFILE_T(IFS* iR)
    {
        ifs_PTR        = iR;
        Search.Section = nullptr;
        Search.Key     = nullptr;
    }

    ~INIFILE_T() override;

    SEARCH_DATA Search;

    IFS* ifs_PTR;

    // add string to file
    void AddString(char const* section_name, char const* key_name, char const* string) override;
    // write string to file, overwrite data if exist, throw EXS exception object if failed
    void WriteString(char const* section_name, char const* key_name, char const* string) override;
    // write int32_t value of key in pointed section if section and key exist, throw EXS object otherwise
    void WriteLong(char const* section_name, char const* key_name, int32_t value) override;
    // write double value of key in pointed section if section and key exist, throw EXS object otherwise
    void WriteDouble(char const* section_name, char const* key_name, double value) override;

    // fill buffer with key value, throw EXS exception object if failed or if section or key doesnt exist
    void ReadString(char const* section_name, char const* key_name, char* buffer, size_t buffer_size) override;
    // fill buffer with key value if section and key exist, otherwise fill with def_string and return false
    bool ReadString(char const* section_name, char const* key_name, char* buffer, size_t buffer_size, char const* def_string) override;
    // continue search from key founded in previous call this function or to function ReadString
    // fill buffer with key value if section and key exist, otherwise return false
    bool ReadStringNext(char const* section_name, char const* key_name, char* buffer, size_t buffer_size) override;

    // return int32_t value of key in pointed section if section and key exist, throw EXS object otherwise
    int32_t GetInt(char const* section_name, char const* key_name) override;
    // return int32_t value of key in pointed section if section and key exist, if not - return def_value
    int32_t GetInt(char const* section_name, char const* key_name, int32_t def_val) override;

    // return double value of key in pointed section if section and key exist, throw EXS object otherwise
    double GetDouble(char const* section_name, char const* key_name) override;
    // return double value of key in pointed section if section and key exist, if not - return def_value
    double GetDouble(char const* section_name, char const* key_name, double def_val) override;

    bool GetIntNext(char const* section_name, char const* key_name, int32_t* val) override;
    bool GetDoubleNext(char const* section_name, char const* key_name, double* val) override;

    float GetFloat(char const* section_name, char const* key_name) override;
    float GetFloat(char const* section_name, char const* key_name, float def_val) override;
    bool  GetFloatNext(char const* section_name, char const* key_name, float* val) override;

    void DeleteKey(char const* section_name, char const* key_name) override;

    void DeleteKey(char const* section_name, char const* key_name, char const* key_value) override;

    void DeleteSection(char const* section_name) override;

    bool TestKey(char const* section_name, char const* key_name, char const* key_value) override;

    bool GetSectionName(char* section_name_buffer, int32_t buffer_size) override;

    bool GetSectionNameNext(char* section_name_buffer, int32_t buffer_size) override;

    void Flush() override;

    bool Reload() override;

    bool CaseSensitive(bool v) override;

    bool TestSection(char const* section_name) override;
};

class FileService: public IFileService
{
protected:
    IFS*     m_opened_files[_MAX_OPEN_INI_FILES];
    uint32_t m_files_count;
    uint32_t m_max_file_index;

private:
    storm::PathsInfo m_paths;

    bool m_use_lowercase;

public:
    FileService();
    ~FileService() override;

    std::filesystem::path transform_path(std::filesystem::path const& path) override;

    std::vector<std::string> string_paths_by_mask(
        std::filesystem::path const& path,
        std::string const&           mask,
        bool                         get_paths,
        bool                         only_dirs  = false,
        bool                         only_files = true,
        bool                         recursive  = false) override;
    std::vector<std::filesystem::path> paths_by_mask(
        std::filesystem::path const& path,
        std::string const&           mask,
        bool                         get_paths,
        bool                         only_dirs  = false,
        bool                         only_files = true,
        bool                         recursive  = false) override;
    std::time_t to_time_t(std::filesystem::file_time_type tp) override;
    std::string executable_directory() const override;

    std::filesystem::path current_path() const override;
    void                  current_path(std::filesystem::path const& path) override;
    bool                  create_directories(std::filesystem::path const& path) override;
    void                  remove(std::filesystem::path const& path) override;
    std::uintmax_t        remove_all(std::filesystem::path const& path) override;
    bool                  read_file_to_mem(std::filesystem::path const& file_path, std::vector<char>& out_buffer) override;

    uintmax_t                       file_size(std::filesystem::path const& file_path) override;
    bool                            exists(std::filesystem::path const& path) override;
    std::filesystem::file_time_type last_write_time(std::filesystem::path const& path) override;

    uint64_t              path_fingerprint(std::filesystem::path const& path) override;
    std::filesystem::path base_directory_path(BaseDirectory dir) const override;

    void init_from_main_config() override;

    // ini files section
    void                     close_ini_files();
    std::unique_ptr<INIFILE> create_ini_file(std::filesystem::path const& file_path, bool fail_if_exist) override;
    std::unique_ptr<INIFILE> open_ini_file(std::filesystem::path const& file_path) override;
    void                     ref_decrement(INIFILE* ini_obj);
    void                     flush_ini_files();
};
