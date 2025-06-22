#pragma once

#include <cassert>
#include <memory>
#include <tuple>
#include <type_traits>

namespace storm
{

template <typename... Args>
class Container
{
public:
    template <typename... Derives>
    constexpr explicit Container(std::shared_ptr<Derives> const&... args) : m_args {args...}
    {
        assert(get<Args>() || ...);
    }

    virtual ~Container() = default;

    template <class C>
        requires(std::is_same_v<C, Args> || ...)
    constexpr std::shared_ptr<C> const& get() const
    {
        return std::get<std::shared_ptr<C>>(m_args);
    }

protected:
    std::tuple<std::shared_ptr<Args>...> m_args;
};

}  // namespace storm
