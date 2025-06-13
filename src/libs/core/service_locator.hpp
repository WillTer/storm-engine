#pragma once

#include <cassert>
#include <memory>
#include <type_traits>
#include <utility>

#include <libs/config/i_config_loader.h>

class COLLIDE;
class VGEOMETRY;
class VSoundService;

namespace storm
{

template <typename... Args>
class ServiceLocatorBase
{
public:
    explicit ServiceLocatorBase(std::shared_ptr<Args>&&... args) : m_services {args...}
    {
        assert((args && ...));  // all pointers must be not null
    }

    template <class C, typename T = std::remove_cvref_t<C>>
        requires((std::is_same_v<T, Args> || ...))
    constexpr std::shared_ptr<T> const& get() const
    {
        return std::get<std::shared_ptr<T>>(m_services);
    }

private:
    std::tuple<std::shared_ptr<Args>...> m_services;
};

using ServiceLocator = ServiceLocatorBase<IConfigLoader, VSoundService, COLLIDE, VGEOMETRY>;

}  // namespace storm
