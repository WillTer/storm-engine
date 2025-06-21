#pragma once

#include <functional>

namespace storm
{

class Defer
{
public:
    Defer(std::function<void()>&& func) : m_func {std::move(func)} {}
    ~Defer()
    {
        m_func();
    }

private:
    std::function<void()> m_func;
};

}  // namespace storm
