#pragma once

#include <memory>
#include <utility>

#include <libs/config/i_config_loader.h>

class VSoundService;

namespace storm
{

template <class S, class Locator>
concept is_service = requires(std::shared_ptr<Locator> const& sl, S& service) {
    { service.Init(sl) } -> std::convertible_to<bool>;
};

template <typename... Args>
class ServiceLocatorBase: public std::enable_shared_from_this<ServiceLocatorBase<Args...>>
{
    struct Private {
        explicit Private() = default;
    };

public:
    explicit ServiceLocatorBase(Private /*unused*/) {}

    static std::shared_ptr<ServiceLocatorBase> create()
    {
        return std::make_shared<ServiceLocatorBase>(Private());
    }

    template <class C>
        requires((std::is_same_v<C, Args> || ...))
    constexpr std::shared_ptr<C> const& get() const
    {
        return std::get<std::shared_ptr<C>>(m_services);
    }

    template <class C>
        requires is_service<C, ServiceLocatorBase> && ((std::is_same_v<C, Args> || ...))
    std::shared_ptr<C> const& set(std::shared_ptr<C>&& ptr)
    {
        std::get<std::shared_ptr<C>>(m_services) = ptr;
        std::get<std::shared_ptr<C>>(m_services)->Init(std::enable_shared_from_this<ServiceLocatorBase>::shared_from_this());
        return std::get<std::shared_ptr<C>>(m_services);
    }

    template <class C>
        requires(!is_service<C, ServiceLocatorBase>) && ((std::is_same_v<C, Args> || ...))
    constexpr std::shared_ptr<C> const& set(std::shared_ptr<C>&& ptr)
    {
        std::get<std::shared_ptr<C>>(m_services) = ptr;
        return std::get<std::shared_ptr<C>>(m_services);
    }

private:
    std::tuple<std::shared_ptr<Args>...> m_services;
};

using ServiceLocator = ServiceLocatorBase<IConfigLoader, VSoundService>;

}  // namespace storm
