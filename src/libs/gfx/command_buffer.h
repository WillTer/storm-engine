#pragma once

#include <memory>

namespace storm::gfx
{

template <typename Ctx, typename CmdBuffer>
std::unique_ptr<CmdBuffer> acquire_cmd_buffer(Ctx& ctx);

struct ContextSDL;
struct CommandBufferSDL;

template <>
std::unique_ptr<CommandBufferSDL> acquire_cmd_buffer(ContextSDL& ctx);

}  // namespace storm::gfx
