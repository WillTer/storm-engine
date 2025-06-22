#include "file_service.h"

#include <algorithm>
#include <exception>
#include <fstream>
#include <regex>
#include <string>

#include <SDL3/SDL_filesystem.h>
#include <libs/config/i_config_loader.h>
#include <libs/core/core_impl.h>
#include <spdlog/spdlog.h>

#include "default_paths.h"

#define COMMENT ';'
#define SECTION_A '['
#define SECTION_B ']'
#define INI_EQUAL '='
#define VOIDSYMS_NUM 2
#define INI_SIGNATURE ";[SE2IF]"

namespace
{

template <typename DirIterator>
auto iter_directory(DirIterator& it, std::string const& mask, bool get_paths, bool only_dirs, bool only_files)
    -> std::vector<std::filesystem::path>
{
    std::vector<std::filesystem::path> result;

    // Transform wildcard expression to regex:
    // 1. Add \ before every . in original mask (to match exactly dot)
    // 2. Replace all * with .*
    // 3. Replace all ? with .
    auto const mask_regex_str = std::regex_replace(
        std::regex_replace(std::regex_replace(mask, std::regex("\\."), "\\."), std::regex("\\*"), ".*"), std::regex("\\?"), ".");
    std::regex const mask_regex("^" + mask_regex_str + "$");

    for (auto& dir_entry: it) {
        bool is_dir = dir_entry.is_directory();
        if ((only_files && is_dir) || (only_dirs && !is_dir)) { continue; }
        auto const cur_path = dir_entry.path();
        if (mask.empty() || std::regex_match(cur_path.filename().string(), mask_regex)) {
            if (get_paths) {
                result.push_back(cur_path);
            } else {
                result.push_back(cur_path.filename());
            }
        }
    }

    return result;
}

}  // namespace

void FileService::flush_ini_files()
{
    for (uint32_t n = 0; n <= m_max_file_index; n++) {
        if (m_opened_files[n] == nullptr) { continue; }
        m_opened_files[n]->FlushFile();
    }
}

FileService::FileService()
{
    m_files_count    = 0;
    m_max_file_index = 0;
    for (uint32_t n = 0; n < _MAX_OPEN_INI_FILES; n++) {
        m_opened_files[n] = nullptr;
    }

    m_paths.resource   = storm::fs::RESOURCE_DIR_DEFAULT;
    m_paths.program    = storm::fs::PROGRAM_DIR_DEFAULT;
    m_paths.ini        = storm::fs::INI_DIR_DEFAULT;
    m_paths.aliases    = storm::fs::ALIASES_DIR_DEFAULT;
    m_paths.sounds     = storm::fs::SOUNDS_DIR_DEFAULT;
    m_paths.videos     = storm::fs::VIDEOS_DIR_DEFAULT;
    m_paths.animation  = storm::fs::ANIMATION_DIR_DEFAULT;
    m_paths.models     = storm::fs::MODELS_DIR_DEFAULT;
    m_paths.foam       = storm::fs::FOAM_DIR_DEFAULT;
    m_paths.techniques = storm::fs::TECHNIQUES_DIR_DEFAULT;
    m_paths.particles  = storm::fs::PARTICLES_DIR_DEFAULT;
    m_paths.textures   = storm::fs::TEXTURES_DIR_DEFAULT;
    m_paths.sea        = storm::fs::SEA_DIR_DEFAULT;
    m_paths.shaders    = storm::fs::SHADERS_DIR_DEFAULT;
    m_use_lowercase    = false;
}

FileService::~FileService()
{
    close_ini_files();
}

std::filesystem::path FileService::transform_path(std::filesystem::path const& path)
{
    auto path_transformed = path.lexically_normal().string();

    if (m_use_lowercase) {
        auto const exe_path = executable_directory();  // Convert part relative to executable only
        auto       it       = std::mismatch(exe_path.begin(), exe_path.end(), path_transformed.begin(), path_transformed.end()).second;
        std::transform(it, path_transformed.end(), it, [](unsigned char const ch) { return std::tolower(ch); });
    }

    // Always use unix format
    std::replace(path_transformed.begin(), path_transformed.end(), '\\', '/');

    return path_transformed;
}

std::vector<std::string> FileService::string_paths_by_mask(
    std::filesystem::path const& path, std::string const& mask, bool get_paths, bool only_dirs, bool only_files, bool recursive)
{
    std::vector<std::string> result;

    auto const paths = paths_by_mask(path, mask, get_paths, only_dirs, only_files, recursive);
    std::transform(paths.begin(), paths.end(), std::back_inserter(result), [](auto const& p) { return p.string(); });

    return result;
}

std::vector<std::filesystem::path> FileService::paths_by_mask(
    std::filesystem::path const& path, std::string const& mask, bool get_paths, bool only_dirs, bool only_files, bool recursive)
{
    std::filesystem::path const src_path = transform_path(path);

    auto const iter = [&](auto&& it, auto const& ec) -> std::vector<std::filesystem::path> {
        if (ec) {
            spdlog::warn("Failed to open folder \"{}\": {}", src_path.string(), ec.message());
            return {};
        }

        return iter_directory(it, mask, get_paths, only_dirs, only_files);
    };

    std::error_code ec = {};
    return recursive ? iter(std::filesystem::recursive_directory_iterator(src_path, ec), ec)
                     : iter(std::filesystem::directory_iterator(src_path, ec), ec);
}

std::time_t FileService::to_time_t(std::filesystem::file_time_type tp)
{
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(tp - std::filesystem::file_time_type::clock::now() + system_clock::now());
    return system_clock::to_time_t(sctp);
}

std::string FileService::executable_directory() const
{
    char const* path = SDL_GetBasePath();
    if (path == nullptr) { return {}; }

    return path;
}

std::filesystem::path FileService::current_path() const
{
    return std::filesystem::current_path();
}

void FileService::current_path(std::filesystem::path const& path)
{
    std::filesystem::current_path(transform_path(path));
}

bool FileService::create_directories(std::filesystem::path const& path)
{
    return std::filesystem::create_directories(transform_path(path));
}

void FileService::remove(std::filesystem::path const& path)
{
    std::filesystem::remove(transform_path(path));
}

std::uintmax_t FileService::remove_all(std::filesystem::path const& path)
{
    return std::filesystem::remove_all(transform_path(path));
}

uintmax_t FileService::file_size(std::filesystem::path const& file_path)
{
    return std::filesystem::file_size(transform_path(file_path));
}

bool FileService::exists(std::filesystem::path const& path)
{
    return std::filesystem::exists(transform_path(path));
}

std::filesystem::file_time_type FileService::last_write_time(std::filesystem::path const& path)
{
    return std::filesystem::last_write_time(transform_path(path));
}

//------------------------------------------------------------------------------------------------
// inifile objects managment
//

std::unique_ptr<INIFILE> FileService::create_ini_file(std::filesystem::path const& file_path, bool fail_if_exist)
{
    if (fio->exists(file_path) && fail_if_exist) { return nullptr; }

    auto stream = open_file<std::ofstream>(file_path, std::ios::binary);
    if (!stream.is_open()) {
        spdlog::error("Can't create ini file: {}", file_path.string());
        return nullptr;
    }
    stream.close();

    return open_ini_file(file_path);
}

std::unique_ptr<INIFILE> FileService::open_ini_file(std::filesystem::path const& file_path)
{
    for (uint32_t i = 0; i <= m_max_file_index; i++) {
        if (m_opened_files[i] == nullptr) { continue; }
        if (m_opened_files[i]->GetFileName() == file_path) {
            m_opened_files[i]->IncReference();

            auto v = std::make_unique<INIFILE_T>(m_opened_files[i]);
            if (!v) { throw std::runtime_error("Failed to create INIFILE_T"); }
            return v;
        }
    }

    for (auto n = 0; n < _MAX_OPEN_INI_FILES; n++) {
        if (m_opened_files[n] != nullptr) { continue; }

        m_opened_files[n] = new IFS(this);
        if (m_opened_files[n] == nullptr) { throw std::runtime_error("Failed to create IFS"); }
        if (!m_opened_files[n]->LoadFile(file_path)) {
            delete m_opened_files[n];
            m_opened_files[n] = nullptr;
            return nullptr;
        }
        m_max_file_index = std::max<uint32_t>(m_max_file_index, n);
        m_opened_files[n]->IncReference();

        auto v = std::make_unique<INIFILE_T>(m_opened_files[n]);
        if (!v) { throw std::runtime_error("Failed to create INIFILE_T"); }
        return v;
    }

    return nullptr;
}

void FileService::ref_decrement(INIFILE* ini_obj)
{
    for (uint32_t n = 0; n <= m_max_file_index; n++) {
        if (m_opened_files[n] != ini_obj) { continue; }
        if (m_opened_files[n]->GetReference() == 0) { throw std::runtime_error("Reference error"); }
        m_opened_files[n]->DecReference();
        if (m_opened_files[n]->GetReference() == 0) {
            delete m_opened_files[n];
            m_opened_files[n] = nullptr;
        }
        return;
    }

    throw std::runtime_error("bad inifile object");
}

void FileService::close_ini_files()
{
    for (uint32_t n = 0; n < _MAX_OPEN_INI_FILES; n++) {
        if (m_opened_files[n] == nullptr) { continue; }
        delete m_opened_files[n];
        m_opened_files[n] = nullptr;
    }
}

bool FileService::read_file_to_mem(std::filesystem::path const& file_path, std::vector<char>& out_buffer)
{
    auto stream = open_file<std::ifstream>(file_path, std::ios::binary);
    if (!stream.is_open()) {
        spdlog::trace("Can't load file: {}", file_path.string());
        return false;
    }

    auto const size = fio->file_size(file_path);
    if (size == 0) { return false; }

    out_buffer.resize(size);
    stream.read(out_buffer.data(), out_buffer.size());

    return true;
}

//------------------------------------------------------------------------------------------------
// Resource paths
//

uint64_t FileService::path_fingerprint(std::filesystem::path const& path)
{
    if (!exists(path)) { return 0; }

    auto const path_transformed = transform_path(path);
    auto const fingerprint      = [](auto const& file) {
        return static_cast<uint64_t>(std::filesystem::last_write_time(file).time_since_epoch().count());
    };

    if (is_regular_file(path_transformed)) { return fingerprint(path_transformed); }

    if (!is_directory(path_transformed)) { return 0; }

    uint64_t timestamp = 0;
    for (auto const& entry: std::filesystem::recursive_directory_iterator(path_transformed)) {
        if (is_regular_file(entry)) { timestamp = std::max(timestamp, fingerprint(entry)); }
    }

    return timestamp;
}

std::filesystem::path FileService::base_directory_path(BaseDirectory dir) const
{
    switch (dir) {
    case BaseDirectory::Resource: return m_paths.resource;
    case BaseDirectory::Program: return m_paths.program;
    case BaseDirectory::Ini: return m_paths.ini;
    case BaseDirectory::Aliases: return m_paths.aliases;
    case BaseDirectory::Sounds: return m_paths.sounds;
    case BaseDirectory::Videos: return m_paths.videos;
    case BaseDirectory::Animation: return m_paths.animation;
    case BaseDirectory::Models: return m_paths.models;
    case BaseDirectory::Foam: return m_paths.foam;
    case BaseDirectory::Techniques: return m_paths.techniques;
    case BaseDirectory::Particles: return m_paths.particles;
    case BaseDirectory::Textures: return m_paths.textures;
    case BaseDirectory::Sea: return m_paths.sea;
    case BaseDirectory::Shaders: return m_paths.shaders;
    case BaseDirectory::None: return std::filesystem::path();
    }

    return executable_directory();
}

void FileService::init_from_main_config()
{
    auto const compat = storm::main_config::compatibility_info();
    m_use_lowercase   = compat.use_lowercase_paths;

    m_paths = storm::main_config::paths_info();
}

//=================================================================================================

INIFILE_T::~INIFILE_T()
{
    if (auto* file_service = dynamic_cast<FileService*>(fio.get()); file_service) {
        try {
            file_service->ref_decrement(ifs_PTR);
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
