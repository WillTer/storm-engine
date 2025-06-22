#include "raylib_window.h"

#include <raylib.h>

namespace storm
{
RaylibWindow::RaylibWindow(int width, int height, int preferred_display, bool fullscreen, [[maybe_unused]] bool bordered)
    : m_width {width}
    , m_height {height}
    , m_title {"Sea Dogs"}
{
    int flags = 0;
    if (!bordered) { flags |= FLAG_WINDOW_UNDECORATED; }
    SetConfigFlags(flags);

    InitWindow(m_width, m_height, m_title.c_str());
    SetWindowMonitor(preferred_display);
    if (IsWindowFullscreen() != fullscreen) { ToggleFullscreen(); }
}

RaylibWindow::~RaylibWindow()
{
    CloseWindow();
}

void RaylibWindow::Show()
{
    ClearWindowState(FLAG_WINDOW_HIDDEN);
}

void RaylibWindow::Hide()
{
    SetWindowState(FLAG_WINDOW_HIDDEN);
}

void RaylibWindow::Focus()
{
    SetWindowFocused();
}

int RaylibWindow::Width() const
{
    return m_width;
}

int RaylibWindow::Height() const
{
    return m_height;
}

WindowSize RaylibWindow::GetWindowSize() const
{
    return {m_width, m_height};
}

bool RaylibWindow::Fullscreen() const
{
    return IsWindowFullscreen();
}

std::string RaylibWindow::Title() const
{
    return m_title;
}

void RaylibWindow::SetFullscreen(bool fullscreen)
{
    if (IsWindowFullscreen() != fullscreen) { ToggleFullscreen(); }
}

void RaylibWindow::Resize(int width, int height)
{
    SetWindowSize(width, height);
}

void RaylibWindow::WarpMouseInWindow(int x, int y) {}

void RaylibWindow::SetTitle(std::string const& title)
{
    m_title = title;
    SetWindowTitle(m_title.c_str());
}

void RaylibWindow::SetGamma(uint16_t const (&red)[256], uint16_t const (&green)[256], uint16_t const (&blue)[256]) {}

int RaylibWindow::Subscribe(EventHandler const& handler)
{
    int id = 1;
    if (!m_handlers.empty()) { id = (--m_handlers.end())->first + 1; }
    m_handlers[id] = handler;
    m_handlers[id](Event::FocusGained);
    return id;
}

void RaylibWindow::Unsubscribe(int id)
{
    auto it = m_handlers.find(id);
    if (it != m_handlers.end()) { m_handlers.erase(it); }
}

void* RaylibWindow::OSHandle()
{
    return GetWindowHandle();
}

std::shared_ptr<IWindow> IWindow::Create(int width, int height, int preferred_display, bool fullscreen, bool bordered)
{
    return std::make_shared<RaylibWindow>(width, height, preferred_display, fullscreen, bordered);
}

}  // namespace storm
