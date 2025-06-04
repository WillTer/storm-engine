#pragma once

#include <map>

#include <SDL2/SDL.h>
#include <libs/input/input.hpp>
#include <libs/util/platform/platform.hpp>

namespace storm
{
class SDLInput: public Input
{
public:
    SDLInput();
    ~SDLInput() override;

    int  Subscribe(EventHandler const& handler) override;
    void Unsubscribe(int id) override;

    bool KeyboardModState(KeyboardKey const& key) const override;
    bool KeyboardKeyState(KeyboardKey const& key) const override;
    bool KeyboardSDLKeyState(SDL_Scancode const& key) const override;
    bool MouseKeyState(MouseKey const& key) const override;

    uint32_t GetWheelFactor() const override;

    bool ControllerButtonState(ControllerButton const& button) const override;
    int  ControllerAxisValue(ControllerAxis const& axis) const override;

private:
    static int SDLCALL SDLEventHandler(void* userdata, SDL_Event* evt);
    void               ProcessEvent(SDL_Event const& event);
    void               OpenController();

private:
    std::map<int, EventHandler>                                                   handlers_;
    uint8_t const*                                                                keyStates_;
    std::unique_ptr<SDL_GameController, std::function<void(SDL_GameController*)>> controller_;
    SDL_JoystickID                                                                joyID_;
};
}  // namespace storm
