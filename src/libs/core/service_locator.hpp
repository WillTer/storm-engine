#pragma once

#include <memory>
#include <utility>

#include <libs/config/i_config_loader.h>

namespace storm
{

template <typename... Args>
class ServiceLocatorBase
{
public:
    template <class C>
        requires((std::is_same_v<C, Args> || ...))
    std::shared_ptr<C> const& get() const
    {
        return std::get<std::shared_ptr<C>>(m_services);
    }

    template <class C>
        requires((std::is_same_v<C, Args> || ...))
    void add(std::shared_ptr<C> const& ptr)
    {
        std::get<std::shared_ptr<C>>(m_services) = ptr;
    }

private:
    std::tuple<std::shared_ptr<Args>...> m_services;
};

using ServiceLocator = ServiceLocatorBase<IConfigLoader>;

}  // namespace storm
