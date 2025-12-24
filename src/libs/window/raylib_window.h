#pragma once

#include <map>

#include "os_window.hpp"

namespace storm
{
class RaylibWindow: public OSWindow
{
public:
    RaylibWindow(int width, int height, int preferred_display, bool fullscreen, bool bordered);
    ~RaylibWindow() override;

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
    void SetTitle(std::string const& title) override;
    void SetGamma(uint16_t const (&red)[256], uint16_t const (&green)[256], uint16_t const (&blue)[256]) override;

    int  Subscribe(EventHandler const& handler) override;
    void Unsubscribe(int id) override;

    void* OSHandle() override;

private:
    int                         m_width;
    int                         m_height;
    std::string                 m_title;
    std::map<int, EventHandler> m_handlers;
};
}  // namespace storm
