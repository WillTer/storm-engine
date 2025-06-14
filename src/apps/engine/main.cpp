#include <thread>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <entt/entity/organizer.hpp>
#include <entt/entity/registry.hpp>
#include <libs/collide/vcollide.h>
#include <libs/config/config_loader.h>
#include <libs/config/main_config.h>
#include <libs/core/core_private.h>
#include <libs/diagnostics/lifecycle_diagnostics_service.hpp>
#include <libs/diagnostics/logging.hpp>
#include <libs/diagnostics/watermark.hpp>
#include <libs/geometry/geometry_r.h>
#include <libs/sound_service/sound_service.h>
#include <libs/steam_api/steam_api.hpp>
#include <libs/util/fs.h>
#include <libs/window/os_window.hpp>
#include <spdlog/spdlog.h>

namespace
{

constexpr char DEFAULT_LOGGER_NAME[] = "system";

storm::diag::LifecycleDiagnosticsService lifecycle_diagnostics;

bool run_frame(CorePrivate& core)
{
    bool const is_running = core.Run();
    lifecycle_diagnostics.notifyAfterRun();

    return is_running;
}

#ifdef _WIN32
bool run_frame_with_overflow_check(CorePrivate& core)
{
    bool is_running = false;
    __try {
        is_running = run_frame(core);
    } __except ([](unsigned code, struct _EXCEPTION_POINTERS* ep) {
        return code == EXCEPTION_STACK_OVERFLOW;
    }(GetExceptionCode(), GetExceptionInformation())) {
        _resetstkoflw();
        throw std::runtime_error("Stack overflow");
    }

    return is_running;
}
#else
bool run_frame_with_overflow_check(CorePrivate& core)
{
    return run_frame(core);
}
#endif

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

    auto const registry = std::make_shared<entt::registry>();

    auto const config_loader = std::make_unique<storm::ConfigLoader>(*fio);
    auto const sound_service = std::make_unique<SoundService>();
    auto const collide       = std::make_unique<COLL>();
    // auto const geometry      = std::make_unique<GEOMETRY>();

    registry->ctx().emplace<storm::IConfigLoader&>(*config_loader);
    registry->ctx().emplace<VSoundService&>(*sound_service).Init(registry);
    registry->ctx().emplace<COLLIDE&>(*collide).Init(registry);
    // registry->ctx().emplace<VGEOMETRY&>(*geometry).Init(registry);

    entt::organizer execute_start;
    execute_start.emplace<&sound_service::run_start>("VSoundService::RunStart");

    entt::organizer execute_end;
    execute_end.emplace<&sound_service::run_end>("VSoundService::RunEnd");

    auto& core_private = static_cast<CorePrivate&>(core);

    // Load parameters of file service
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
        lifecycle_diagnostics.setCrashInfoCollector([&core_private]() { core_private.collectCrashInfo(); });
    }

    // Init stash
    create_directories(fs::GetSaveDataPath());

    // Init logging
    spdlog::set_default_logger(storm::logging::getOrCreateLogger(DEFAULT_LOGGER_NAME));
    spdlog::info("Logging system initialized. Running on {}", STORM_BUILD_WATERMARK);

    // Init core
    core_private.Init(registry);

    // Read config
    auto const general_info = storm::main_config::general_info(*config_loader);
    auto const window_info  = storm::main_config::window_info(*config_loader);

    if (!general_info.enable_logs)  // disable logging
    {
        spdlog::set_level(spdlog::level::off);
    }

    bool const is_sound_in_background_enabled = window_info.run_in_background && window_info.sound_in_background;
    // initialize SteamApi through evaluating its singleton
    try {
        steamapi::SteamApi::getInstance(!general_info.use_steam);
    } catch (std::exception const& e) {
        spdlog::critical(e.what());
        return EXIT_FAILURE;
    }

    auto const window = storm::OSWindow::Create(
        window_info.width, window_info.height, window_info.preferred_display, window_info.full_screen, window_info.show_borders);
    window->SetTitle("Sea Dogs");
    window->Show();
    core_private.SetWindow(window);

    // Init core
    core_private.InitBase();

    core_private.set_organizer_for_section_start(SECTION_EXECUTE, execute_start);
    core_private.set_organizer_for_section_end(SECTION_EXECUTE, execute_end);

    // Message loop
    auto old_time = SDL_GetTicks();

    bool should_close = false;
    bool is_active    = true;
    while (!should_close) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev) > 0) {
            switch (ev.type) {
                // Window Events
            case SDL_QUIT: should_close = true; break;
            case SDL_WINDOWEVENT:
                switch (ev.window.event) {
                case SDL_WINDOWEVENT_CLOSE: should_close = true; break;
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    is_active = true;
                    core_private.AppState(is_active);
                    if (!is_sound_in_background_enabled) { sound_service->set_active_with_fade(true); }
                    break;
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    is_active = false;
                    core_private.AppState(is_active);
                    if (!is_sound_in_background_enabled) { sound_service->set_active_with_fade(false); }
                    break;
                }
                break;
            }
        }

        if (is_active || window_info.run_in_background) {
            if (window_info.max_fps != 0U) {
                auto const ms       = 1000U / window_info.max_fps;
                auto const new_time = SDL_GetTicks();
                if (new_time - old_time < ms) { continue; }
                old_time = new_time;
            }

            should_close = !run_frame_with_overflow_check(core_private);
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    // Release
    core_private.Event("DestroyWindow");
    core_private.Event("ExitApplication");
    core_private.CleanUp();
    core_private.ReleaseBase();
#ifdef _WIN32  // FIX_LINUX Cursor
    ClipCursor(nullptr);
#endif
    SDL_Quit();

    return EXIT_SUCCESS;
}
