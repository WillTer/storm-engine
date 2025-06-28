#pragma once

#include <memory>

struct SDL_Window;

namespace storm
{

class AssetServer;
class IConfigLoader;

namespace gfx
{

template <typename C>
auto create_context(std::shared_ptr<AssetServer> const& asset_server, std::shared_ptr<IConfigLoader> const& config_loader)
    -> std::unique_ptr<C>;

template <typename C, typename RawWindow>
void claim_window(C& ctx, RawWindow raw_window);

template <typename C, typename RawWindow>
void unclaim_window(C& ctx, RawWindow raw_window);

struct ContextSDL;

template <>
auto create_context(std::shared_ptr<AssetServer> const& asset_server, std::shared_ptr<IConfigLoader> const& config_loader)
    -> std::unique_ptr<ContextSDL>;

template <>
void claim_window(ContextSDL& ctx, std::shared_ptr<SDL_Window> const& raw_window);

template <>
void unclaim_window(ContextSDL& ctx, std::shared_ptr<SDL_Window> const& raw_window);

}  // namespace gfx

}  // namespace storm
