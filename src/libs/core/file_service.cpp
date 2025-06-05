#include "file_service.h"

#include <algorithm>
#include <exception>
#include <fstream>
#include <string>

#include <SDL2/SDL.h>
#include <libs/util/platform/platform.hpp>
#include <libs/util/string_compare.hpp>

#include "core_impl.h"

#define COMMENT ';'
#define SECTION_A '['
#define SECTION_B ']'
#define INI_EQUAL '='
#define VOIDSYMS_NUM 2
#define INI_SIGNATURE ";[SE2IF]"

namespace
{
FILE_SERVICE file_service;
}

VFILE_SERVICE* fio = &file_service;

void FILE_SERVICE::FlushIniFiles()
{
    for (uint32_t n = 0; n <= Max_File_Index; n++) {
        if (OpenFiles[n] == nullptr) { continue; }
        OpenFiles[n]->FlushFile();
    }
}

FILE_SERVICE::FILE_SERVICE()
{
    Files_Num      = 0;
    Max_File_Index = 0;
    for (uint32_t n = 0; n < _MAX_OPEN_INI_FILES; n++) {
        OpenFiles[n] = nullptr;
    }
}

FILE_SERVICE::~FILE_SERVICE()
{
    Close();
}

std::fstream FILE_SERVICE::_CreateFile(std::filesystem::path const& file_path, std::ios::openmode mode)
{
    std::fstream stream(file_path, mode);
    return stream;
}

void FILE_SERVICE::_CloseFile(std::fstream& stream)
{
    stream.close();
}

void FILE_SERVICE::_SetFilePointer(std::fstream& fileS, std::streamoff off, std::ios::seekdir dir)
{
    fileS.seekp(off, dir);
}

bool FILE_SERVICE::_DeleteFile(std::filesystem::path const& file_path)
{
    return std::filesystem::remove(file_path);
}

bool FILE_SERVICE::_WriteFile(std::fstream& fileS, void const* s, std::streamsize count)
{
    fileS.exceptions(std::fstream::failbit | std::fstream::badbit);
    try {
        fileS.write(reinterpret_cast<char const*>(s), count);
        return true;
    } catch (std::fstream::failure const& e) {
        spdlog::error("Failed to WriteFile: {}", e.what());
        return false;
    }
}

bool FILE_SERVICE::_ReadFile(std::fstream& fileS, void* s, std::streamsize count)
{
    fileS.exceptions(std::fstream::failbit | std::fstream::badbit);
    try {
        fileS.read(reinterpret_cast<char*>(s), count);
        return true;
    } catch (std::fstream::failure const& e) {
        spdlog::error("Failed to ReadFile: {}", e.what());
        return false;
    }
}

bool FILE_SERVICE::_FileOrDirectoryExists(std::filesystem::path const& path)
{
    auto ec     = std::error_code {};
    bool result = std::filesystem::exists(path, ec);
    if (ec) {
        spdlog::error("Failed to to check if {} exists: {}", path.string(), ec.message());
        return false;
    }

    return result;
}

std::vector<std::string> FILE_SERVICE::_GetPathsOrFilenamesByMask(
    std::filesystem::path const& path, char const* mask, bool getPaths, bool onlyDirs, bool onlyFiles, bool recursive)
{
    std::vector<std::string> result;

    auto const fsPaths = _GetFsPathsByMask(path, mask, getPaths, onlyDirs, onlyFiles, recursive);
    for (std::filesystem::path cur_path: fsPaths) {
        result.emplace_back(cur_path.string());
    }

    return result;
}

template <typename DirIterator>
std::vector<std::filesystem::path>
iter_directory(DirIterator& it, std::error_code& ec, char const* mask, bool getPaths, bool onlyDirs, bool onlyFiles)
{
    std::vector<std::filesystem::path> result;

    if (ec) {
        spdlog::warn("Failed to open save folder: {}", ec.message());
        return result;
    }

    std::filesystem::path curPath;
    for (auto& dirEntry: it) {
        bool thisIsDir = dirEntry.is_directory();
        if ((onlyFiles && thisIsDir) || (onlyDirs && !thisIsDir)) { continue; }
        curPath = dirEntry.path();
        if (mask == nullptr || storm::wildicmp(mask, curPath.filename().string().c_str())) {
            if (getPaths) {
                result.push_back(curPath);
            } else {
                result.push_back(curPath.filename());
            }
        }
    }

    return result;
}

std::vector<std::filesystem::path> FILE_SERVICE::_GetFsPathsByMask(
    std::filesystem::path const& path, char const* mask, bool getPaths, bool onlyDirs, bool onlyFiles, bool recursive)
{
    std::filesystem::path const src_path = std::filesystem::path(path);

    std::error_code ec;
    if (recursive) {
        auto it = std::filesystem::recursive_directory_iterator(src_path, ec);
        return iter_directory(it, ec, mask, getPaths, onlyDirs, onlyFiles);
    }

    auto it = std::filesystem::directory_iterator(src_path, ec);
    return iter_directory(it, ec, mask, getPaths, onlyDirs, onlyFiles);
}

std::time_t FILE_SERVICE::_ToTimeT(std::filesystem::file_time_type tp)
{
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(tp - std::filesystem::file_time_type::clock::now() + system_clock::now());
    return system_clock::to_time_t(sctp);
}

std::filesystem::file_time_type FILE_SERVICE::_GetLastWriteTime(std::filesystem::path const& file_path)
{
    return std::filesystem::last_write_time(file_path);
}

void FILE_SERVICE::_FlushFileBuffers(std::fstream& fileS)
{
    fileS.flush();
}

std::string FILE_SERVICE::_GetCurrentDirectory()
{
    auto const  curPath = std::filesystem::current_path().string();
    std::string result(curPath.begin(), curPath.end());
    return result;
}

std::string FILE_SERVICE::_GetExecutableDirectory()
{
    char*             path   = SDL_GetBasePath();
    std::string const result = path;
    SDL_free(path);

    return result;
}

std::uintmax_t FILE_SERVICE::_GetFileSize(std::filesystem::path const& file_path)
{
    return std::filesystem::file_size(file_path);
}

void FILE_SERVICE::_SetCurrentDirectory(std::filesystem::path const& path)
{
    std::filesystem::current_path(path);
}

bool FILE_SERVICE::_CreateDirectory(std::filesystem::path const& path)
{
    return std::filesystem::create_directories(path);
}

std::uintmax_t FILE_SERVICE::_RemoveDirectory(std::filesystem::path const& path)
{
    return std::filesystem::remove_all(path);
}

//------------------------------------------------------------------------------------------------
// inifile objects managment
//

std::unique_ptr<INIFILE> FILE_SERVICE::CreateIniFile(std::filesystem::path const& file_path, bool fail_if_exist)
{
    if (std::filesystem::exists(file_path) && fail_if_exist) { return nullptr; }

    auto stream = std::fstream(file_path, std::ios::binary | std::ios::out);
    if (!stream.is_open()) {
        spdlog::error("Can't create ini file: {}", file_path.string());
        return nullptr;
    }

    return OpenIniFile(file_path);
}

std::unique_ptr<INIFILE> FILE_SERVICE::OpenIniFile(std::filesystem::path const& file_path)
{
    for (uint32_t i = 0; i <= Max_File_Index; i++) {
        if (OpenFiles[i] == nullptr) { continue; }
        if (OpenFiles[i]->GetFileName() == file_path) {
            OpenFiles[i]->IncReference();

            auto v = std::make_unique<INIFILE_T>(OpenFiles[i]);
            if (!v) { throw std::runtime_error("Failed to create INIFILE_T"); }
            return v;
        }
    }

    for (auto n = 0; n < _MAX_OPEN_INI_FILES; n++) {
        if (OpenFiles[n] != nullptr) { continue; }

        OpenFiles[n] = new IFS(this);
        if (OpenFiles[n] == nullptr) { throw std::runtime_error("Failed to create IFS"); }
        if (!OpenFiles[n]->LoadFile(file_path)) {
            delete OpenFiles[n];
            OpenFiles[n] = nullptr;
            return nullptr;
        }
        Max_File_Index = std::max<uint32_t>(Max_File_Index, n);
        OpenFiles[n]->IncReference();

        auto v = std::make_unique<INIFILE_T>(OpenFiles[n]);
        if (!v) { throw std::runtime_error("Failed to create INIFILE_T"); }
        return v;
    }

    return nullptr;
}

void FILE_SERVICE::RefDec(INIFILE* ini_obj)
{
    for (uint32_t n = 0; n <= Max_File_Index; n++) {
        if (OpenFiles[n] != ini_obj) { continue; }
        if (OpenFiles[n]->GetReference() == 0) { throw std::runtime_error("Reference error"); }
        OpenFiles[n]->DecReference();
        if (OpenFiles[n]->GetReference() == 0) {
            delete OpenFiles[n];
            OpenFiles[n] = nullptr;
        }
        return;
    }

    throw std::runtime_error("bad inifile object");
}

void FILE_SERVICE::Close()
{
    for (uint32_t n = 0; n < _MAX_OPEN_INI_FILES; n++) {
        if (OpenFiles[n] == nullptr) { continue; }
        delete OpenFiles[n];
        OpenFiles[n] = nullptr;
    }
}

bool FILE_SERVICE::LoadFile(std::filesystem::path const& file_path, std::vector<char>& out_buffer)
{
    auto stream = std::fstream(file_path, std::ios::binary | std::ios::in);
    if (!stream.is_open()) {
        spdlog::trace("Can't load file: {}", file_path.string());
        return false;
    }

    auto const size = std::filesystem::file_size(file_path);
    if (size == 0) { return false; }

    out_buffer.resize(size);
    stream.read(out_buffer.data(), out_buffer.size());

    return true;
}

//------------------------------------------------------------------------------------------------
// Resource paths
//

void terminate_with_char(std::string& buffer, char const chr)
{
    // Check if already has
    if (!buffer.empty() && buffer[buffer.length() - 1] != chr) {
        // Append to end and store
        buffer += chr;
    }
}

std::string get_dir_iterator_path(std::filesystem::path const& path)
{
    std::string path_str = path.string();
    size_t      pos      = path_str.find(std::string(".") + PATH_SEP);
    if (pos != std::string::npos && pos == 0) { path_str.erase(0, 2); }
    return path_str;
}

void string_replace(std::string& input, char const* find, char const* paste)
{
    size_t pos = 0;
    while (true) {
        pos = input.find(find, pos);
        if (pos >= input.size()) break;
        input.replace(pos, strlen(find), paste);
        pos += strlen(paste);
    }
}

std::string convert_path(char const* path)
{
    std::string conv;
    size_t      size = strlen(path);
    for (int i = 0; i < size; ++i) {
        conv.push_back(path[i] == WRONG_PATH_SEP ? PATH_SEP : path[i]);
    }
    return conv;
}

uint64_t FILE_SERVICE::GetPathFingerprint(std::filesystem::path const& path)
{
    if (!exists(path)) { return 0; }

    auto const fingerprint = [](auto const& file) {
        return static_cast<uint64_t>(std::filesystem::last_write_time(file).time_since_epoch().count());
    };

    if (is_regular_file(path)) { return fingerprint(path); }

    if (!is_directory(path)) { return 0; }

    uint64_t timestamp = 0;
    for (auto const& entry: std::filesystem::recursive_directory_iterator(path)) {
        if (is_regular_file(entry)) { timestamp = std::max(timestamp, fingerprint(entry)); }
    }

    return timestamp;
}

//=================================================================================================

INIFILE_T::~INIFILE_T()
{
    if (auto* file_service = dynamic_cast<FILE_SERVICE*>(fio); file_service) {
        try {
            file_service->RefDec(ifs_PTR);
        } catch (std::exception const& e) {
            spdlog::error(e.what());
        }
    }
}

void INIFILE_T::AddString(char const* section_name, char const* key_name, char const* string)
{
    ifs_PTR->AddString(section_name, key_name, string);
}

// write string to file, overwrite data if exist, throw EXS exception object if failed
void INIFILE_T::WriteString(char const* section_name, char const* key_name, char const* string)
{
    ifs_PTR->WriteString(section_name, key_name, string);
}

// write int32_t value of key in pointed section if section and key exist, throw EXS object otherwise
void INIFILE_T::WriteLong(char const* section_name, char const* key_name, int32_t value)
{
    ifs_PTR->WriteLong(section_name, key_name, value);
}

// write double value of key in pointed section if section and key exist, throw EXS object otherwise
void INIFILE_T::WriteDouble(char const* section_name, char const* key_name, double value)
{
    ifs_PTR->WriteDouble(section_name, key_name, value);
}

// fill buffer with key value, throw EXS exception object if failed or if section or key doesnt exist
void INIFILE_T::ReadString(char const* section_name, char const* key_name, char* buffer, size_t buffer_size)
{
    ifs_PTR->ReadString(&Search, section_name, key_name, buffer, buffer_size);
}

// fill buffer with key value if section and key exist, otherwise fill with def_string and return false
bool INIFILE_T::ReadString(char const* section_name, char const* key_name, char* buffer, size_t buffer_size, char const* def_string)
{
    return ifs_PTR->ReadString(&Search, section_name, key_name, buffer, buffer_size, def_string);
}

// continue search from key founded in previous call this function or to function ReadString
// fill buffer with key value if section and key exist, otherwise return false
bool INIFILE_T::ReadStringNext(char const* section_name, char const* key_name, char* buffer, size_t buffer_size)
{
    return ifs_PTR->ReadStringNext(&Search, section_name, key_name, buffer, buffer_size);
}

// return int32_t value of key in pointed section if section and key exist, throw EXS object otherwise
int32_t INIFILE_T::GetInt(char const* section_name, char const* key_name)
{
    return ifs_PTR->GetInt(&Search, section_name, key_name);
}

// return int32_t value of key in pointed section if section and key exist, if not - return def_value
int32_t INIFILE_T::GetInt(char const* section_name, char const* key_name, int32_t def_val)
{
    return ifs_PTR->GetInt(&Search, section_name, key_name, def_val);
}

// return double value of key in pointed section if section and key exist, throw EXS object otherwise
double INIFILE_T::GetDouble(char const* section_name, char const* key_name)
{
    return ifs_PTR->GetDouble(&Search, section_name, key_name);
}

// return double value of key in pointed section if section and key exist, if not - return def_value
double INIFILE_T::GetDouble(char const* section_name, char const* key_name, double def_val)
{
    return ifs_PTR->GetDouble(&Search, section_name, key_name, def_val);
}

bool INIFILE_T::GetIntNext(char const* section_name, char const* key_name, int32_t* val)
{
    return ifs_PTR->GetIntNext(&Search, section_name, key_name, val);
}

bool INIFILE_T::GetDoubleNext(char const* section_name, char const* key_name, double* val)
{
    return ifs_PTR->GetDoubleNext(&Search, section_name, key_name, val);
}

// return double value of key in pointed section if section and key exist, throw EXS object otherwise
float INIFILE_T::GetFloat(char const* section_name, char const* key_name)
{
    return ifs_PTR->GetFloat(&Search, section_name, key_name);
}

// return float value of key in pointed section if section and key exist, if not - return def_value
float INIFILE_T::GetFloat(char const* section_name, char const* key_name, float def_val)
{
    return ifs_PTR->GetFloat(&Search, section_name, key_name, def_val);
}

bool INIFILE_T::GetFloatNext(char const* section_name, char const* key_name, float* val)
{
    return ifs_PTR->GetFloatNext(&Search, section_name, key_name, val);
}

void INIFILE_T::DeleteKey(char const* section_name, char const* key_name)
{
    ifs_PTR->DeleteKey(section_name, key_name);
}

void INIFILE_T::DeleteKey(char const* section_name, char const* key_name, char const* key_value)
{
    ifs_PTR->DeleteKey(section_name, key_name, key_value);
}

bool INIFILE_T::TestKey(char const* section_name, char const* key_name, char const* key_value)
{
    return ifs_PTR->TestKey(section_name, key_name, key_value);
}

void INIFILE_T::DeleteSection(char const* section_name)
{
    ifs_PTR->DeleteSection(section_name);
}

bool INIFILE_T::GetSectionName(char* section_name_buffer, int32_t buffer_size)
{
    return ifs_PTR->GetSectionName(section_name_buffer, buffer_size);
}

bool INIFILE_T::GetSectionNameNext(char* section_name_buffer, int32_t buffer_size)
{
    return ifs_PTR->GetSectionNameNext(section_name_buffer, buffer_size);
}

void INIFILE_T::Flush()
{
    ifs_PTR->Flush();
}

bool INIFILE_T::Reload()
{
    return ifs_PTR->Reload();
}

bool INIFILE_T::CaseSensitive(bool v)
{
    return ifs_PTR->CaseSensitive(v);
}

bool INIFILE_T::TestSection(char const* section_name)
{
    return ifs_PTR->TestSection(section_name);
};
