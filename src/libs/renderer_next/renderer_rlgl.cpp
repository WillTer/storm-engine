#include "renderer_rlgl.h"

#include <format>
#include <memory>
#include <stdexcept>

using namespace storm;

namespace
{
#ifdef _DEBUG
constexpr bool IS_DEBUG_MODE = true;
#else
constexpr bool IS_DEBUG_MODE = false;
#endif
}  // namespace

RendererRlgl::RendererRlgl() {}

RendererRlgl::~RendererRlgl() = default;

void RendererRlgl::init() {}
