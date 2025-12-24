#include <thread>

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <libs/config/config_loader.h>
#include <libs/config/main_config.h>
#include <libs/core/core_impl.h>
#include <libs/core/core_private.h>
#include <libs/core/vma.hpp>
#include <libs/diagnostics/logging.hpp>
#include <libs/diagnostics/watermark.hpp>
#include <libs/filesystem/file_service.h>
#include <libs/renderer_next/renderer_next.h>
#include <libs/sound_service/v_sound_service.h>
#include <libs/steam_api/steam_api.hpp>
#include <libs/util/fs.h>
#include <libs/window/os_window.hpp>
#include <spdlog/spdlog.h>

std::unique_ptr<IFileService>           fio              = nullptr;
std::unique_ptr<storm::ClassesRegistry> classes_registry = nullptr;  // Only for linking, initialized in another place (vma.hpp)
std::shared_ptr<CoreImpl>               core_internal    = nullptr;
std::shared_ptr<Core>                   core             = nullptr;
std::unique_ptr<storm::IConfigLoader>   config_loader    = nullptr;

namespace
{

constexpr char DEFAULT_LOGGER_NAME[]          = "engine";
bool           is_sound_in_background_enabled = false;
bool           is_active                      = false;
bool           should_close                   = false;

bool run_frame()
{
    bool const is_running = core_internal->Run();

    return is_running;
}

#ifdef _WIN32
bool run_frame_with_overflow_check()
{
    bool is_running = false;
    __try {
        is_running = run_frame();
    } __except ([](unsigned code, struct _EXCEPTION_POINTERS* ep) {
        return code == EXCEPTION_STACK_OVERFLOW;
    }(GetExceptionCode(), GetExceptionInformation())) {
        _resetstkoflw();
        throw std::runtime_error("Stack overflow");
    }

    return is_running;
}
#else
bool run_frame_with_overflow_check()
{
    return run_frame();
}
#endif

void handle_window_event(storm::OSWindow::Event const& event)
{
    if (event == storm::OSWindow::Closed) {
        should_close = true;
        if (core_internal->initialized()) { core_internal->Event("DestroyWindow"); }
    } else if (event == storm::OSWindow::FocusGained) {
        is_active = true;
        if (core_internal->initialized()) {
            core_internal->AppState(is_active);
            if (auto* const sound_service = static_cast<VSoundService*>(core->GetService("SoundService"));
                (sound_service != nullptr) && !is_sound_in_background_enabled) {
                sound_service->set_active_with_fade(true);
            }
        }
    } else if (event == storm::OSWindow::FocusLost) {
        is_active = false;
        if (core_internal->initialized()) {
            core_internal->AppState(is_active);
            if (auto* const sound_service = static_cast<VSoundService*>(core->GetService("SoundService"));
                (sound_service != nullptr) && !is_sound_in_background_enabled) {
                sound_service->set_active_with_fade(false);
            }
        }
    }
}

void release()
{
    if (core_internal) {
        core_internal->Event("ExitApplication");
        core_internal->CleanUp();
        core_internal->ReleaseBase();
    }

#ifdef _WIN32  // FIX_LINUX Cursor
    ClipCursor(nullptr);
#endif

    SDL_Quit();
}

}  // namespace

int main()
try {
    // Prevent multiple instances
#ifdef _WIN32  // CreateEventA
    if (!CreateEventA(nullptr, false, false, "Global\\FBBD2286-A9F1-4303-B60C-743C3D7AA7BE") || GetLastError() == ERROR_ALREADY_EXISTS) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Another instance is already running!", nullptr);
        return EXIT_SUCCESS;
    }
#endif

    setlocale(LC_ALL, "en_US.utf8");  // Enable UTF-8

    SDL_InitSubSystem(SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_GAMEPAD);

    auto renderer = std::make_shared<storm::RendererNext>();

    fio           = std::make_unique<FileService>();
    core_internal = std::make_shared<CoreImpl>(std::move(renderer));
    core          = core_internal;
    config_loader = std::make_unique<storm::ConfigLoader>();

    // Init logging
    storm::logging::init_logger_for_sdl();
    spdlog::set_default_logger(storm::logging::get_logger_with_stdout(DEFAULT_LOGGER_NAME));
    spdlog::flush_every(std::chrono::seconds(3));

    auto const general_info = storm::main_config::general_info();
    if (general_info.enable_logs) {
        spdlog::info("Logging system initialized. Running on {}", STORM_BUILD_WATERMARK);
    } else {  // disable logging
        spdlog::info("Logging disabled!");
        spdlog::set_level(spdlog::level::off);
    }

    // Load parameters of file service
    fio->init_from_main_config();

    // Init stash
    create_directories(fs::GetSaveDataPath());

    // Init core
    core_internal->Init();

    // Read config
    auto const window_info = storm::main_config::window_info();

    is_sound_in_background_enabled = window_info.run_in_background && window_info.sound_in_background;
    // initialize SteamApi through evaluating its singleton
    steamapi::SteamApi::getInstance(!general_info.use_steam);

    std::shared_ptr<storm::OSWindow> window = storm::OSWindow::Create(
        window_info.width, window_info.height, window_info.preferred_display, window_info.full_screen, window_info.show_borders);
    window->SetTitle("Sea Dogs");
    window->Subscribe(handle_window_event);
    window->Show();
    core_internal->SetWindow(window);

    // Init core
    core_internal->InitBase();

    // Message loop
    auto old_time = SDL_GetTicks();

    bool is_running = true;
    while (is_running && !should_close) {
        SDL_PumpEvents();
        SDL_FlushEvents(0, SDL_EVENT_LAST);

        if (is_active || window_info.run_in_background) {
            if (window_info.max_fps != 0U) {
                auto const ms       = 1000U / window_info.max_fps;
                auto const new_time = SDL_GetTicks();
                if (new_time - old_time < ms) { continue; }
                old_time = new_time;
            }

            is_running = run_frame_with_overflow_check();
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    release();
    return EXIT_SUCCESS;
} catch (std::runtime_error const& e) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Engine error", e.what(), nullptr);
    spdlog::critical(e.what());

    release();
    return EXIT_FAILURE;
}
