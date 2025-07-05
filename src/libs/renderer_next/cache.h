#pragma once

#include <any>
#include <cassert>
#include <functional>
#include <memory>
#include <unordered_map>

#include <entt/core/hashed_string.hpp>
#include <entt/core/type_info.hpp>
#include <libs/util/hashed_string_map.h>

namespace storm
{

class RendererCache final
{
public:
    template <typename Obj>
    void add(entt::hashed_string const& name, std::shared_ptr<Obj> const& obj)
    {
        assert(obj);

        auto const obj_type = entt::type_id<Obj>().index();
        if (!m_objects.contains(obj_type)) { m_objects.emplace(obj_type, id_type_map<std::any> {}); }

        auto& obj_map = m_objects.at(obj_type);
        if (obj_map.contains(name.value())) {
            obj_map[name.value()] = obj;
        } else {
            obj_map.emplace(name.value(), obj);
        }
    }

    template <typename Obj>
    auto get(entt::hashed_string const& name) const -> std::shared_ptr<std::remove_cvref_t<Obj>>
    {
        auto const obj_type = entt::type_id<Obj>().index();
        if (!m_objects.contains(obj_type)) { return nullptr; }

        auto const& obj_map = m_objects.at(obj_type);
        if (!obj_map.contains(name.value())) { return nullptr; }

        return std::any_cast<std::shared_ptr<std::remove_cvref_t<Obj>>>(obj_map.at(name.value()));
    }

    template <typename Obj>
    bool contains(entt::hashed_string const& name) const
    {
        auto const obj_type = entt::type_id<Obj>().index();
        if (!m_objects.contains(obj_type)) { return false; }

        auto const& obj_map = m_objects.at(obj_type);
        return obj_map.contains(name.value());
    }

    template <typename Obj>
    void remove(entt::hashed_string const& name)
    {
        auto const obj_type = entt::type_id<Obj>().index();
        if (!m_objects.contains(obj_type)) { return; }

        auto& obj_map = m_objects.at(obj_type);
        if (obj_map.contains(name.value())) { obj_map.erase(name.value()); }
    }

    template <typename Obj>
    void clear_by_type()
    {
        auto const obj_type = entt::type_id<Obj>().index();
        if (!m_objects.contains(obj_type)) { return; }

        m_objects.at(obj_type).clear();
    }

    void clear()
    {
        m_objects.clear();
    }

    template <typename Obj>
    void for_each(std::function<void(Obj&)> const& pred)
    {
        auto const obj_type = entt::type_id<Obj>().index();
        if (!m_objects.contains(obj_type)) { return; }

        auto& obj_map = m_objects.at(obj_type);
        std::for_each(obj_map.begin(), obj_map.end(), [&pred](auto const& pair) {
            pred(*std::any_cast<std::shared_ptr<std::remove_cvref_t<Obj>>>(pair.second));
        });
    }

private:
    id_type_map<id_type_map<std::any>> m_objects;
};

}  // namespace storm
