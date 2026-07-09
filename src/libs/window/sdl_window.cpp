#include "sdl_window.hpp"

#include <libs/renderer_next/impl_sdl/renderer_sdl.h>

namespace storm
{
SDLWindow::SDLWindow(std::shared_ptr<RendererService> const& renderer, WindowInfo const& info)
    : m_renderer(renderer)
    , m_fullscreen(info.full_screen)
{
    auto const props = SDL_CreateProperties();
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, SDL_WINDOWPOS_CENTERED_DISPLAY(info.preferred_display));
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, SDL_WINDOWPOS_CENTERED_DISPLAY(info.preferred_display));
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, info.width);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, info.height);
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN, m_fullscreen);
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_HIDDEN_BOOLEAN, true);

    m_handle = std::unique_ptr<SDL_Window, std::function<void(SDL_Window*)>>(
        SDL_CreateWindowWithProperties(props), [](SDL_Window* w) { SDL_DestroyWindow(w); });

    SDL_DestroyProperties(props);

    m_window_id = SDL_GetWindowID(m_handle.get());
    SDL_SetWindowBordered(m_handle.get(), info.show_borders);
    SDL_AddEventWatch(&SDLEventHandler, this);

    m_renderer->bind_window(m_handle);
}

SDLWindow::~SDLWindow()
{
    m_renderer->unbind_window(m_handle);
    SDL_RemoveEventWatch(&SDLEventHandler, this);
}

void SDLWindow::Show()
{
    SDL_ShowWindow(m_handle.get());
}

void SDLWindow::Hide()
{
    SDL_HideWindow(m_handle.get());
}

void SDLWindow::Focus()
{
    SDL_RaiseWindow(m_handle.get());
}

int SDLWindow::Width() const
{
    return GetWindowSize().width;
}

int SDLWindow::Height() const
{
    return GetWindowSize().height;
}

WindowSize SDLWindow::GetWindowSize() const
{
    int w = 0;
    int h = 0;
    SDL_GetWindowSize(m_handle.get(), &w, &h);
    return {w, h};
}

bool SDLWindow::Fullscreen() const
{
    return m_fullscreen;
}

std::string SDLWindow::Title() const
{
    return std::string(SDL_GetWindowTitle(m_handle.get()));
}

void SDLWindow::SetFullscreen(bool fullscreen)
{
    m_fullscreen = fullscreen;
    SDL_SetWindowFullscreen(m_handle.get(), m_fullscreen);
}

void SDLWindow::Resize(int width, int height)
{
    SDL_SetWindowSize(m_handle.get(), width, height);
}

void SDLWindow::WarpMouseInWindow(int x, int y)
{
    SDL_WarpMouseInWindow(m_handle.get(), static_cast<float>(x), static_cast<float>(y));
}

auto SDLWindow::get_mouse_pos() const -> std::pair<int, int>
{
    float mouse_x = 0.0F;
    float mouse_y = 0.0F;
    SDL_GetMouseState(&mouse_x, &mouse_y);
    return std::make_pair(static_cast<int>(mouse_x), static_cast<int>(mouse_y));
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
    SDL_SetWindowTitle(m_handle.get(), title.c_str());
}

void SDLWindow::SetGamma(uint16_t const (&red)[256], uint16_t const (&green)[256], uint16_t const (&blue)[256])
{
    // Removed in SDL3
    // SDL_SetWindowGammaRamp(window_.get(), red, green, blue);
}

int SDLWindow::Subscribe(EventHandler const& handler)
{
    int id = 1;
    if (!m_handlers.empty()) {
        id = (--m_handlers.end())->first + 1;
    }
    m_handlers[id] = handler;
    return id;
}

void SDLWindow::Unsubscribe(int id)
{
    if (auto it = m_handlers.find(id); it != m_handlers.end()) {
        m_handlers.erase(it);
    }
}

void* SDLWindow::RawHandle()
{
    return m_handle.get();
}

void SDLWindow::ProcessEvent(SDL_WindowEvent const& evt) const
{
    Event win_event = {};
    switch (evt.type) {
    case SDL_EVENT_WINDOW_FOCUS_GAINED: win_event = FocusGained; break;

    case SDL_EVENT_WINDOW_FOCUS_LOST: win_event = FocusLost; break;

    case SDL_EVENT_WINDOW_CLOSE_REQUESTED: win_event = Closed; break;

    default: return;
    }

    for (auto handler: m_handlers) {
        handler.second(win_event);
    }
}

std::shared_ptr<IWindow> IWindow::Create(std::shared_ptr<RendererService> const& renderer, WindowInfo const& info)
{
    return std::make_shared<SDLWindow>(renderer, info);
}

bool SDLWindow::SDLEventHandler(void* userdata, SDL_Event* evt)
{
    auto const* const w = static_cast<SDLWindow*>(userdata);

    if (evt->type < SDL_EVENT_WINDOW_FIRST || evt->type > SDL_EVENT_WINDOW_LAST || evt->window.windowID != w->m_window_id) {
        return false;
    }

    w->ProcessEvent(evt->window);

    return false;
}
}  // namespace storm
