#pragma once

#include <functional>
#include <memory>
#include <string>
#include <utility>

namespace storm
{
struct WindowSize {
    int width {};
    int height {};
};

class RendererService;

//! Abstract window
class IWindow
{
public:
    // TODO: More events?
    enum Event {
        Unknown,
        //!< Invalid event

        FocusGained,
        //!< Window gained focus
        FocusLost,
        //!< Window lost focus
        Closed  //!< Window was closed by user
    };

    using EventHandler = std::function<void(Event const&)>;

    virtual ~IWindow() {};

    //! Show window
    virtual void Show() = 0;
    //! Hide window
    virtual void Hide() = 0;
    //! Make window focused
    virtual void Focus() = 0;

    //! Current width of window
    virtual int Width() const = 0;
    //! Current height of window
    virtual int Height() const = 0;
    //! Current window size
    virtual WindowSize GetWindowSize() const = 0;
    //! Is window fullscreen
    virtual bool Fullscreen() const = 0;
    //! Window title
    virtual std::string Title() const = 0;

    //! Toggle fullscreen
    virtual void SetFullscreen(bool fullscreen) = 0;
    //! Resize window
    virtual void Resize(int width, int height) = 0;
    //! Warp mouse
    virtual void WarpMouseInWindow(int x, int y) = 0;
    //! Mouse position
    virtual auto get_mouse_pos() const -> std::pair<int, int> = 0;
    //! Toggle mouse cursor
    virtual void show_cursor(bool is_shown) = 0;
    //! Set window title
    virtual void SetTitle(std::string const& title) = 0;
    //! Set window gamma
    virtual void SetGamma(uint16_t const (&red)[256], uint16_t const (&green)[256], uint16_t const (&blue)[256]) = 0;

    //! Subscribe for events
    //! \param handler event callback
    //! \return subscription id, which should be passed to unsubscribe()
    virtual int Subscribe(EventHandler const& handler) = 0;
    //! Unsubscribe from events
    //! \param id handler returned by subscribe()
    virtual void Unsubscribe(int id) = 0;

    //! Os-depended window handler (i.e. HWND on Windows)
    virtual void* OSHandle() = 0;

    //! Create new window
    static std::shared_ptr<IWindow>
    Create(std::shared_ptr<RendererService> const& renderer, int width, int height, int preferred_display, bool fullscreen, bool bordered);
};
}  // namespace storm
