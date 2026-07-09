#pragma once

#include <map>

#include <SDL3/SDL.h>

#include "os_window.hpp"

namespace storm
{
class SDLWindow: public IWindow
{
public:
    SDLWindow(std::shared_ptr<RendererService> const& renderer, WindowInfo const& info);
    ~SDLWindow() override;

    void Show() override;
    void Hide() override;
    void Focus() override;

    int         Width() const override;
    int         Height() const override;
    WindowSize  GetWindowSize() const override;
    bool        Fullscreen() const override;
    std::string Title() const override;

    void SetFullscreen(bool fullscreen) override;
    void Resize(int width, int height) override;
    void WarpMouseInWindow(int x, int y) override;
    auto get_mouse_pos() const -> std::pair<int, int> override;
    void show_cursor(bool is_shown) override;
    void SetTitle(std::string const& title) override;
    void SetGamma(uint16_t const (&red)[256], uint16_t const (&green)[256], uint16_t const (&blue)[256]) override;

    int  Subscribe(EventHandler const& handler) override;
    void Unsubscribe(int id) override;

    void* RawHandle() override;

    void ProcessEvent(SDL_WindowEvent const& evt) const;

private:
    static bool SDLCALL SDLEventHandler(void* userdata, SDL_Event* evt);

    std::shared_ptr<SDL_Window>      m_handle   = nullptr;
    std::shared_ptr<RendererService> m_renderer = nullptr;

    uint32_t                    m_window_id;
    bool                        m_fullscreen = false;
    std::map<int, EventHandler> m_handlers;
};
}  // namespace storm
