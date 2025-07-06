#include "sdl_window.hpp"

#include <libs/renderer_next/impl_sdl/renderer_sdl.h>

namespace storm
{
SDLWindow::SDLWindow(
    std::shared_ptr<RendererService> const& renderer, int width, int height, int preferred_display, bool fullscreen, bool bordered)
    : m_renderer(renderer)
    , fullscreen_(fullscreen)
{
    auto const props = SDL_CreateProperties();
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, SDL_WINDOWPOS_CENTERED_DISPLAY(preferred_display));
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, SDL_WINDOWPOS_CENTERED_DISPLAY(preferred_display));
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, width);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, height);
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN, fullscreen);
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_HIDDEN_BOOLEAN, true);

    window_ = std::unique_ptr<SDL_Window, std::function<void(SDL_Window*)>>(
        SDL_CreateWindowWithProperties(props), [](SDL_Window* w) { SDL_DestroyWindow(w); });

    SDL_DestroyProperties(props);

    sdlID_ = SDL_GetWindowID(window_.get());
    SDL_SetWindowBordered(window_.get(), bordered);
    SDL_AddEventWatch(&SDLEventHandler, this);

    m_renderer->bind_window(window_);
}

SDLWindow::~SDLWindow()
{
    m_renderer->unbind_window(window_);
    SDL_RemoveEventWatch(&SDLEventHandler, this);
}

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
    SDL_WarpMouseInWindow(window_.get(), static_cast<float>(x), static_cast<float>(y));
}

auto SDLWindow::get_mouse_pos() const -> std::pair<float, float>
{
    float mouse_x = 0.0F;
    float mouse_y = 0.0F;
    SDL_GetMouseState(&mouse_x, &mouse_y);
    return std::make_pair(mouse_x, mouse_y);
}

void SDLWindow::show_cursor(bool is_shown)
{
    if (is_shown) {
        SDL_ShowCursor();
    } else {
        SDL_HideCursor();
    }
}

void SDLWindow::SetTitle(std::string const& title)
{
    SDL_SetWindowTitle(window_.get(), title.c_str());
}

void SDLWindow::SetGamma(uint16_t const (&red)[256], uint16_t const (&green)[256], uint16_t const (&blue)[256])
{
    // Removed in SDL3
    // SDL_SetWindowGammaRamp(window_.get(), red, green, blue);
}

int SDLWindow::Subscribe(EventHandler const& handler)
{
    int id = 1;
    if (!handlers_.empty()) id = (--handlers_.end())->first + 1;
    handlers_[id] = handler;
    return id;
}

void SDLWindow::Unsubscribe(int id)
{
    auto it = handlers_.find(id);
    if (it != handlers_.end()) handlers_.erase(it);
}

// TODO: X/Wayland/MacOS
void* SDLWindow::OSHandle()
{
    if (!window_) return nullptr;

#ifdef _WIN32
    return SDL_GetPointerProperty(SDL_GetWindowProperties(window_.get()), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
#else
    // dxvk-native uses HWND as SDL2 window handle, so this is allowed
    return window_.get();
#endif
}

SDL_Window* SDLWindow::SDLHandle() const
{
    return window_.get();
}

void SDLWindow::ProcessEvent(SDL_WindowEvent const& evt) const
{
    Event winEvent;
    switch (evt.type) {
    case SDL_EVENT_WINDOW_FOCUS_GAINED: winEvent = FocusGained; break;

    case SDL_EVENT_WINDOW_FOCUS_LOST: winEvent = FocusLost; break;

    case SDL_EVENT_WINDOW_CLOSE_REQUESTED: winEvent = Closed; break;

    default: return;
    }

    for (auto handler: handlers_)
        handler.second(winEvent);
}

std::shared_ptr<IWindow> IWindow::Create(
    std::shared_ptr<RendererService> const& renderer, int width, int height, int preferred_display, bool fullscreen, bool bordered)
{
    return std::make_shared<SDLWindow>(renderer, width, height, preferred_display, fullscreen, bordered);
}

bool SDLWindow::SDLEventHandler(void* userdata, SDL_Event* evt)
{
    auto const* const w = static_cast<SDLWindow*>(userdata);

    if (evt->type < SDL_EVENT_WINDOW_FIRST || evt->type > SDL_EVENT_WINDOW_LAST || evt->window.windowID != w->sdlID_) return false;

    w->ProcessEvent(evt->window);

    return false;
}
}  // namespace storm
