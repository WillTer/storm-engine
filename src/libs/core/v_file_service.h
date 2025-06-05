#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

class INIFILE;

enum class BaseDirectory {
    None,
    Resource,
    Ini,
    Aliases,
    Sounds,
    Videos,
    Animation,
    Models,
    Foam,
    Techniques,
    Particles,
    Textures,
    Sea,
};

class IFileService
{
public:
    virtual ~IFileService() = default;

    template <typename Stream = std::fstream>
    Stream open_file(std::filesystem::path const& file_path, std::ios::openmode mode)
    {
        return Stream(transform_path(file_path), mode);
    }

    virtual std::filesystem::path transform_path(std::filesystem::path const& path) = 0;

    virtual std::vector<std::string> string_paths_by_mask(
        std::filesystem::path const& source_path,
        std::string const&           mask,
        bool                         get_paths,
        bool                         only_dirs  = false,
        bool                         only_files = true,
        bool                         recursive  = false) = 0;
    virtual std::vector<std::filesystem::path> paths_by_mask(
        std::filesystem::path const& source_path,
        std::string const&           mask,
        bool                         get_paths,
        bool                         only_dirs  = false,
        bool                         only_files = true,
        bool                         recursive  = false)                                                                                    = 0;
    virtual std::time_t    to_time_t(std::filesystem::file_time_type tp)                                           = 0;
    virtual std::string    executable_directory()                                                                  = 0;
    virtual void           set_current_directory(std::filesystem::path const& path)                                = 0;
    virtual bool           create_directory(std::filesystem::path const& path)                                     = 0;
    virtual std::uintmax_t remove_directory(std::filesystem::path const& path)                                     = 0;
    virtual bool           read_file_to_mem(std::filesystem::path const& file_path, std::vector<char>& out_buffer) = 0;

    // Update IFileService internal variables according to configuration
    virtual void load_service_parameters_from_config(std::filesystem::path const& config_file) = 0;

    // ini files section
    virtual std::unique_ptr<INIFILE> create_ini_file(std::filesystem::path const& file, bool fail_if_exist) = 0;
    virtual std::unique_ptr<INIFILE> open_ini_file(std::filesystem::path const& file)                       = 0;

    virtual uint64_t path_fingerprint(std::filesystem::path const& path) = 0;

    virtual std::filesystem::path base_directory_path(BaseDirectory dir) = 0;
};

//------------------------------------------------------------------------------------------------
// handle with text files in format:
//
// ; commentary
// [section name 1]
// key_name1 = string1
// ...
// key_nameN = stringN
// ...
// [section name N]
// ...
// spacebar and tab symbols ignored:
// for key_name: from line start to first significant symbol and from end of key_name to '=' symbol
// for string: from symbol '=' to first significant symbol and at the end of line
//
// If section name is null, functions search in all keys

class INIFILE
{
public:
    virtual ~INIFILE() = default;

    // add string to file
    virtual void AddString(char const* section_name, char const* key_name, char const* string) = 0;
    // write string to file, overwrite data if exist, throw EXS exception object if failed
    virtual void WriteString(char const* section_name, char const* key_name, char const* string) = 0;
    // write int32_t value of key in pointed section if section and key exist, throw EXS object otherwise
    virtual void WriteLong(char const* section_name, char const* key_name, int32_t value) = 0;
    // write double value of key in pointed section if section and key exist, throw EXS object otherwise
    virtual void WriteDouble(char const* section_name, char const* key_name, double value) = 0;

    // fill buffer with key value, throw EXS exception object if failed or if section or key doesnt exist
    virtual void ReadString(char const* section_name, char const* key_name, char* buffer, size_t buffer_size) = 0;

    // fill buffer with key value if section and key exist, otherwise fill with def_string and return false
    virtual bool ReadString(char const* section_name, char const* key_name, char* buffer, size_t buffer_size, char const* def_string) = 0;

    // continue search from key founded in previous call this function or to function ReadString
    // fill buffer with key value if section and key exist, otherwise return false
    virtual bool ReadStringNext(char const* section_name, char const* key_name, char* buffer, size_t buffer_size) = 0;

    // return int32_t value of key in pointed section if section and key exist, throw EXS object otherwise
    virtual int32_t GetInt(char const* section_name, char const* key_name) = 0;
    // return int32_t value of key in pointed section if section and key exist, if not - return def_value
    virtual int32_t GetInt(char const* section_name, char const* key_name, int32_t def_val) = 0;
    // continue scanning for key in section, fill val with int32_t value of key if it found and return true
    // if not - return false
    virtual bool GetIntNext(char const* section_name, char const* key_name, int32_t* val) = 0;

    // return double value of key in pointed section if section and key exist, throw EXS object otherwise
    virtual double GetDouble(char const* section_name, char const* key_name) = 0;
    // return double value of key in pointed section if section and key exist, if not - return def_value
    virtual double GetDouble(char const* section_name, char const* key_name, double def_val) = 0;
    // continue scanning for key in section, fill val with double value of key if it found and return true
    // if not - return false
    virtual bool GetDoubleNext(char const* section_name, char const* key_name, double* val) = 0;

    virtual float GetFloat(char const* section_name, char const* key_name)                 = 0;
    virtual float GetFloat(char const* section_name, char const* key_name, float def_val)  = 0;
    virtual bool  GetFloatNext(char const* section_name, char const* key_name, float* val) = 0;

    virtual std::string GetString(char const* section_name, char const* key_name)                             = 0;
    virtual std::string GetString(char const* section_name, char const* key_name, std::string const& def_val) = 0;

    // virtual void    SetSearch(void *)= 0;

    virtual void DeleteKey(char const* section_name, char const* key_name) = 0;

    virtual void DeleteKey(char const* section_name, char const* key_name, char const* key_value) = 0;

    virtual void DeleteSection(char const* section_name) = 0;

    virtual bool TestKey(char const* section_name, char const* key_name, char const* key_value) = 0;

    virtual bool GetSectionName(char* section_name_buffer, int32_t buffer_size)     = 0;
    virtual bool GetSectionNameNext(char* section_name_buffer, int32_t buffer_size) = 0;

    virtual void Flush()                               = 0;
    virtual bool Reload()                              = 0;
    virtual bool CaseSensitive(bool yes)               = 0;
    virtual bool TestSection(char const* section_name) = 0;
};

//
extern IFileService* fio;
