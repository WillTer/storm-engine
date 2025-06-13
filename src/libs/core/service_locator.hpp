#pragma once

#include <memory>
#include <type_traits>
#include <utility>

#include <libs/config/i_config_loader.h>

class COLLIDE;
class VSoundService;

namespace storm
{

template <class S, class Locator>
concept has_init_with_locator = requires(std::shared_ptr<Locator> const& sl, S& service) {
    { service.Init(sl) } -> std::convertible_to<bool>;
};

// TODO: Move to some local type_traits analogue
template <typename T>
struct base_type {
    using type = T;
};

template <typename Tp, typename... List>
struct find_base_in_pack {
    using type = Tp;
};

template <typename Tp, typename Head, typename... Rest>
struct find_base_in_pack<Tp, Head, Rest...>
    : std::conditional_t<std::is_base_of_v<Head, Tp>, base_type<Head>, find_base_in_pack<Tp, Rest...>> {
};

template <typename Tp, typename... Rest>
using find_base_in_pack_t = typename find_base_in_pack<Tp, Rest...>::type;

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

    template <class C, typename T = std::remove_cvref_t<C>>
        requires((std::is_same_v<T, Args> || ...))
    constexpr std::shared_ptr<T> const& get() const
    {
        if (!std::get<std::shared_ptr<T>>(m_services)) { throw std::runtime_error("Service object is not set"); }

        return std::get<std::shared_ptr<T>>(m_services);
    }

    template <class C, typename T = find_base_in_pack_t<std::remove_cvref_t<C>, Args...>>
        requires((std::is_same_v<T, Args> || ...) && has_init_with_locator<T, ServiceLocatorBase>)
    std::shared_ptr<T> const& set(std::shared_ptr<C>&& ptr)
    {
        std::get<std::shared_ptr<T>>(m_services) = std::move(ptr);
        std::get<std::shared_ptr<T>>(m_services)->Init(std::enable_shared_from_this<ServiceLocatorBase>::shared_from_this());
        return std::get<std::shared_ptr<T>>(m_services);
    }

    template <class C, typename T = find_base_in_pack_t<std::remove_cvref_t<C>, Args...>>
        requires((std::is_same_v<T, Args> || ...) && (!has_init_with_locator<T, ServiceLocatorBase>))
    constexpr std::shared_ptr<T> const& set(std::shared_ptr<C>&& ptr)
    {
        std::get<std::shared_ptr<T>>(m_services) = std::move(ptr);
        return std::get<std::shared_ptr<T>>(m_services);
    }

private:
    std::tuple<std::shared_ptr<Args>...> m_services;
};

using ServiceLocator = ServiceLocatorBase<IConfigLoader, VSoundService, COLLIDE>;

}  // namespace storm
