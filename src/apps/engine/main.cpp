#include <thread>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <libs/config/config_loader.h>
#include <libs/config/main_config.h>
#include <libs/core/core_private.h>
#include <libs/core/service_locator.hpp>
#include <libs/diagnostics/lifecycle_diagnostics_service.hpp>
#include <libs/diagnostics/logging.hpp>
#include <libs/diagnostics/watermark.hpp>
#include <libs/filesystem/default_paths.h>
#include <libs/sound_service/v_sound_service.h>
#include <libs/steam_api/steam_api.hpp>
#include <libs/util/fs.h>
#include <libs/window/os_window.hpp>
#include <spdlog/spdlog.h>

namespace
{

CorePrivate* core_private;

constexpr char DEFAULT_LOGGER_NAME[]          = "system";
bool           is_sound_in_background_enabled = false;
bool           is_active                      = false;
bool           should_close                   = false;

storm::diag::LifecycleDiagnosticsService lifecycle_diagnostics;

bool run_frame()
{
    bool const is_running = core_private->Run();
    lifecycle_diagnostics.notifyAfterRun();

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
#define run_frame_with_overflow_check run_frame
#endif

void handle_window_event(storm::OSWindow::Event const& event)
{
    if (event == storm::OSWindow::Closed) {
        should_close = true;
        if (core_private->initialized()) { core_private->Event("DestroyWindow"); }
    } else if (event == storm::OSWindow::FocusGained) {
        is_active = true;
        if (core_private->initialized()) {
            core_private->AppState(is_active);
            if (auto* const sound_service = static_cast<VSoundService*>(core.GetService("SoundService"));
                (sound_service != nullptr) && !is_sound_in_background_enabled) {
                sound_service->set_active_with_fade(true);
            }
        }
    } else if (event == storm::OSWindow::FocusLost) {
        is_active = false;
        if (core_private->initialized()) {
            core_private->AppState(is_active);
            if (auto* const sound_service = static_cast<VSoundService*>(core.GetService("SoundService"));
                (sound_service != nullptr) && !is_sound_in_background_enabled) {
                sound_service->set_active_with_fade(false);
            }
        }
    }
}

}  // namespace

int main()
{
    // Prevent multiple instances
#ifdef _WIN32  // CreateEventA
    if (!CreateEventA(nullptr, false, false, "Global\\FBBD2286-A9F1-4303-B60C-743C3D7AA7BE") || GetLastError() == ERROR_ALREADY_EXISTS) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Another instance is already running!", nullptr);
        return EXIT_SUCCESS;
    }
#endif

    setlocale(LC_ALL, "en_US.utf8");  // Enable UTF-8

    auto const service_locator = std::make_shared<storm::ServiceLocator>();
    service_locator->add<storm::IConfigLoader>(std::make_shared<storm::ConfigLoader>(*fio));

    // Load parameters of file service
    auto const config_loader = service_locator->get<storm::IConfigLoader>();
    fio->init_from_main_config(*config_loader);

    SDL_InitSubSystem(SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);

    // Init diagnostics
    auto const lifecycle_diagnostics_guard =
#ifdef STORM_ENABLE_CRASH_REPORTS
        lifecycle_diagnostics.initialize(true);
#else
        lifecycle_diagnostics.initialize(false);
#endif
    if (!lifecycle_diagnostics_guard) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Warning", "Unable to initialize lifecycle service!", nullptr);
    } else {
        lifecycle_diagnostics.setCrashInfoCollector([]() { core_private->collectCrashInfo(); });
    }

    // Init stash
    create_directories(fs::GetSaveDataPath());

    // Init logging
    spdlog::set_default_logger(storm::logging::getOrCreateLogger(DEFAULT_LOGGER_NAME));
    spdlog::info("Logging system initialized. Running on {}", STORM_BUILD_WATERMARK);

    // Init core
    core_private = static_cast<CorePrivate*>(&core);
    core_private->Init(service_locator);

    // Read config
    auto const general_info = storm::main_config::general_info(*config_loader);
    auto const window_info  = storm::main_config::window_info(*config_loader);

    if (!general_info.enable_logs)  // disable logging
    {
        spdlog::set_level(spdlog::level::off);
    }

    is_sound_in_background_enabled = window_info.run_in_background && window_info.sound_in_background;
    // initialize SteamApi through evaluating its singleton
    try {
        steamapi::SteamApi::getInstance(!general_info.use_steam);
    } catch (std::exception const& e) {
        spdlog::critical(e.what());
        return EXIT_FAILURE;
    }

    std::shared_ptr<storm::OSWindow> window = storm::OSWindow::Create(
        window_info.width, window_info.height, window_info.preferred_display, window_info.full_screen, window_info.show_borders);
    window->SetTitle("Sea Dogs");
    window->Subscribe(handle_window_event);
    window->Show();
    core_private->SetWindow(window);

    // Init core
    core_private->InitBase();

    // Message loop
    auto old_time = SDL_GetTicks();

    bool is_running = true;
    while (is_running && !should_close) {
        SDL_PumpEvents();
        SDL_FlushEvents(0, SDL_LASTEVENT);

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

    // Release
    core_private->Event("ExitApplication");
    core_private->CleanUp();
    core_private->ReleaseBase();
#ifdef _WIN32  // FIX_LINUX Cursor
    ClipCursor(nullptr);
#endif
    SDL_Quit();

    return EXIT_SUCCESS;
}
