#include "sdl_window.hpp"

#include <SDL2/SDL_syswm.h>

namespace storm
{
SDLWindow::SDLWindow(int width, int height, int preferred_display, bool fullscreen, bool bordered) : fullscreen_ {fullscreen}
{
    uint32_t flags = (fullscreen ? SDL_WINDOW_FULLSCREEN : 0) | SDL_WINDOW_HIDDEN;
#if !defined(_WIN32) && !defined(STORM_MESA_NINE)  // DXVK-Native
    flags |= SDL_WINDOW_VULKAN;
#endif
    window_ = std::shared_ptr<SDL_Window>(
        SDL_CreateWindow(
            "", SDL_WINDOWPOS_CENTERED_DISPLAY(preferred_display), SDL_WINDOWPOS_CENTERED_DISPLAY(preferred_display), width, height, flags),
        &SDL_DestroyWindow);

    sdlID_ = SDL_GetWindowID(window_.get());
    SDL_SetWindowBordered(window_.get(), bordered ? SDL_TRUE : SDL_FALSE);
}

SDLWindow::~SDLWindow() = default;

void SDLWindow::Show()
{
    SDL_ShowWindow(window_.get());
}

void SDLWindow::Hide()
{
    SDL_HideWindow(window_.get());
}

void SDLWindow::Focus()
{
    SDL_RaiseWindow(window_.get());
}

int SDLWindow::Width() const
{
    int w, h;
    SDL_GetWindowSize(window_.get(), &w, &h);
    return w;
}

int SDLWindow::Height() const
{
    int w, h;
    SDL_GetWindowSize(window_.get(), &w, &h);
    return h;
}

WindowSize SDLWindow::GetWindowSize() const
{
    int w, h;
    SDL_GetWindowSize(window_.get(), &w, &h);
    return {w, h};
}

bool SDLWindow::Fullscreen() const
{
    return fullscreen_;
}

std::string SDLWindow::Title() const
{
    return std::string(SDL_GetWindowTitle(window_.get()));
}

void SDLWindow::SetFullscreen(bool fullscreen)
{
    fullscreen_ = fullscreen;
    SDL_SetWindowFullscreen(window_.get(), fullscreen_ ? SDL_WINDOW_FULLSCREEN : 0);
}

void SDLWindow::Resize(int width, int height)
{
    SDL_SetWindowSize(window_.get(), width, height);
}

void SDLWindow::WarpMouseInWindow(int x, int y)
{
    SDL_WarpMouseInWindow(window_.get(), x, y);
}

void SDLWindow::SetTitle(std::string const& title)
{
    SDL_SetWindowTitle(window_.get(), title.c_str());
}

void SDLWindow::SetGamma(uint16_t const (&red)[256], uint16_t const (&green)[256], uint16_t const (&blue)[256])
{
    SDL_SetWindowGammaRamp(window_.get(), red, green, blue);
}

// TODO: X/Wayland/MacOS
void* SDLWindow::OSHandle()
{
    if (!window_) return nullptr;

#ifdef _WIN32
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(window_.get(), &info);
    return info.info.win.window;
#else
    // dxvk-native uses HWND as SDL2 window handle, so this is allowed
    return window_.get();
#endif
}

SDL_Window* SDLWindow::SDLHandle() const
{
    return window_.get();
}

std::shared_ptr<OSWindow> OSWindow::Create(int width, int height, int preferred_display, bool fullscreen, bool bordered)
{
    return std::make_shared<SDLWindow>(width, height, preferred_display, fullscreen, bordered);
}

}  // namespace storm
