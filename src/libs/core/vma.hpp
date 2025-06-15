#pragma once

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include <entt/core/hashed_string.hpp>

/* TODO: REMOVE THIS.... */
constexpr uint32_t case_insensitive_hash(std::string_view const& str)
{
    std::string str_lower = {};
    std::transform(str.begin(), str.end(), std::back_inserter(str_lower), [](unsigned char const c) { return std::tolower(c); });
    return entt::hashed_string::value(str_lower.c_str(), str_lower.size());
}

class VMA;

namespace storm
{

class ClassesRegistry
{
public:
    struct Hasher {
        constexpr std::size_t operator()(entt::id_type const& key) const
        {
            return key;
        }
    };

    auto begin()
    {
        return m_registry.begin();
    }

    auto end()
    {
        return m_registry.end();
    }

    auto cbegin() const
    {
        return m_registry.cbegin();
    }

    auto cend() const
    {
        return m_registry.cend();
    }

    [[nodiscard]] bool contains(entt::id_type const& key) const
    {
        return m_registry.contains(key);
    }

    [[nodiscard]] VMA* at(entt::id_type const& key) const
    {
        return m_registry.at(key);
    }

    void emplace(entt::hashed_string const& key, VMA* value)
    {
        m_registry.emplace(key.value(), value);
    }

private:
    std::unordered_map<entt::id_type, VMA*, Hasher> m_registry;
};

}  // namespace storm

extern std::unique_ptr<storm::ClassesRegistry> classes_registry;

using namespace entt::literals;

class VMA
{
    constexpr static auto vma_name = "VMA"_hs;

protected:
    VMA*    m_next;
    int32_t m_ref_count;

public:
    VMA(entt::hashed_string const& name) : m_next(nullptr)
    {
        m_ref_count = 0;
        if (!classes_registry) { classes_registry = std::make_unique<storm::ClassesRegistry>(); }
        classes_registry->emplace(name, this);
    }

    VMA* next() const
    {
        return m_next;
    }

    virtual ~VMA() = default;

    virtual uint32_t get_hash() const
    {
        return vma_name.value();
    }

    void set_next(VMA* _p)
    {
        m_next = _p;
    }

    virtual bool is_service() const
    {
        return false;
    }

    virtual void* create_class()
    {
        return nullptr;
    }

    virtual void ref_decrement()
    {
        --m_ref_count;
    }

    virtual int32_t get_ref_count() const
    {
        return m_ref_count;
    }

    virtual void clear()
    {
        m_ref_count = 0;
    }

    virtual bool is_library() const
    {
        return false;
    }
};

#define CREATE_CLASS(a) \
    class a##vmacd: public VMA \
    { \
        constexpr static auto m_name = #a##_hs; \
\
    public: \
        a##vmacd() : VMA(m_name) {} \
        uint32_t get_hash() const override \
        { \
            return m_name.value(); \
        } \
        void* create_class() override \
        { \
            ++m_ref_count; \
            return new a; \
        } \
    } a##vmaci;
#define CREATE_SERVICE(a) \
    class a##vmacd: public VMA \
    { \
        constexpr static auto m_name = #a##_hs; \
\
        std::unique_ptr<a> m_service = nullptr; \
\
    public: \
        a##vmacd() : VMA(m_name) {} \
        uint32_t get_hash() const override \
        { \
            return m_name.value(); \
        } \
        void* create_class() override \
        { \
            if (!m_service) { \
                m_service = std::make_unique<a>(); \
                if (!m_service->Init()) { \
                    clear(); \
                    return nullptr; \
                } \
            } \
            ++m_ref_count; \
            return m_service.get(); \
        } \
        bool is_service() const override \
        { \
            return true; \
        } \
        void clear() override \
        { \
            m_ref_count = 0; \
            m_service.reset(); \
        }; \
    } a##vmaci;
#define CREATE_SCRIPT_LIBRARY(a) \
    class a##vmacd: public VMA \
    { \
        constexpr static auto m_name = #a##_hs; \
\
    public: \
        a##vmacd() : VMA(m_name) {} \
        uint32_t get_hash() const override \
        { \
            return m_name.value(); \
        } \
        void* create_class() override \
        { \
            a* obj = new a; \
            obj->Init(); \
            return obj; \
        } \
        bool is_library() const override \
        { \
            return true; \
        } \
    } a##vmaci;
