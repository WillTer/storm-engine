#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <libs/util/storm_assert.h>

class VSTRING_CODEC
{
public:
    VSTRING_CODEC()          = default;
    virtual ~VSTRING_CODEC() = default;

    virtual uint32_t    GetNum()                                   = 0;
    virtual uint32_t    Convert(char const* pString)               = 0;
    virtual uint32_t    Convert(char const* pString, int32_t iLen) = 0;
    virtual char const* Convert(uint32_t code)                     = 0;

    virtual void VariableChanged() = 0;
};

class ATTRIBUTES final
{
    // TODO: remove with another iteration of rewriting this
    friend class COMPILER;

    class LegacyProxy;

public:
    explicit ATTRIBUTES(VSTRING_CODEC& p);
    ATTRIBUTES(const ATTRIBUTES&) = delete;
    ATTRIBUTES(ATTRIBUTES&& other) noexcept;
    ATTRIBUTES& operator=(const ATTRIBUTES&) = delete;
    ATTRIBUTES& operator=(ATTRIBUTES&& other) noexcept;
    ~ATTRIBUTES();

    void                             SetBreak(bool set_break);
    [[nodiscard]] ATTRIBUTES*        GetParent() const;
    bool                             operator==(char const* str) const;
    [[nodiscard]] char const*        GetThisName() const;
    [[nodiscard]] bool               HasValue() const noexcept;
    [[nodiscard]] std::string const& GetValue() const;
    [[nodiscard]] LegacyProxy        GetThisAttr() const;
    void                             SetName(std::string_view const& new_name);
    void                             SetValue(std::string_view const& new_value);
    [[nodiscard]] size_t             GetAttributesNum() const;
    [[nodiscard]] ATTRIBUTES*        GetAttributeClass(std::string_view const& name) const;
    [[nodiscard]] ATTRIBUTES*        GetAttributeClass(uint32_t n) const;
    [[nodiscard]] ATTRIBUTES*        VerifyAttributeClass(std::string_view const& name);
    [[nodiscard]] LegacyProxy        GetAttribute(size_t n) const;
    [[nodiscard]] LegacyProxy        GetAttribute(std::string_view const& name) const;
    [[nodiscard]] char const*        GetAttributeName(size_t n) const;
    [[nodiscard]] uint32_t           GetAttributeAsDword(char const* name = nullptr, uint32_t def = 0) const;
    [[nodiscard]] uintptr_t          GetAttributeAsPointer(char const* name = nullptr, uintptr_t def = 0) const;
    [[nodiscard]] float              GetAttributeAsFloat(char const* name = nullptr, float def = 0) const;
    bool                             SetAttributeUseDword(char const* name, uint32_t val);
    bool                             SetAttributeUseFloat(char const* name, float val);
    ATTRIBUTES&                      CreateAttribute(std::string_view const& name);
    ATTRIBUTES*                      CreateAttribute(std::string_view const& name, char const* attribute);
    size_t                           SetAttribute(std::string_view const& name, std::string_view const& attribute);
    [[nodiscard]] ATTRIBUTES         Copy() const;
    bool                             DeleteAttributeClassX(ATTRIBUTES* pA);
    ATTRIBUTES*                      CreateSubAClass(ATTRIBUTES* pRoot, char const* access_string);
    [[nodiscard]] ATTRIBUTES*        FindAClass(ATTRIBUTES* pRoot, char const* access_string);
    [[nodiscard]] ATTRIBUTES*        GetAttributeClassByCode(uint32_t name_code) const;
    [[nodiscard]] ATTRIBUTES*        VerifyAttributeClassByCode(uint32_t name_code);
    ATTRIBUTES*                      CreateAttribute(uint32_t name_code, std::string_view const& attribute);
    size_t                           SetAttribute(uint32_t name_code, std::string_view const& attribute);
    [[nodiscard]] uint32_t           GetThisNameCode() const noexcept;
    void                             SetNameCode(uint32_t n) noexcept;
    [[nodiscard]] VSTRING_CODEC&     GetStringCodec() const noexcept;

private:
    ATTRIBUTES(VSTRING_CODEC& string_codec, ATTRIBUTES* parent, std::string_view const& name);
    ATTRIBUTES(VSTRING_CODEC& string_codec, ATTRIBUTES* parent, uint32_t name_code);

    void        Release() const noexcept;
    ATTRIBUTES* CreateNewAttribute(uint32_t name_code);

    VSTRING_CODEC&                           stringCodec_;
    uint32_t                                 nameCode_ {};
    std::optional<std::string>               value_;
    std::vector<std::unique_ptr<ATTRIBUTES>> attributes_;
    ATTRIBUTES*                              parent_ {nullptr};
    bool                                     break_ {false};

    class LegacyProxy
    {
        using value_t       = decltype(value_);
        using proxy_value_t = std::add_lvalue_reference_t<std::add_const_t<value_t>>;

    public:
        LegacyProxy(LegacyProxy const& other)            = delete;
        LegacyProxy(LegacyProxy&& other)                 = delete;
        LegacyProxy& operator=(LegacyProxy const& other) = delete;
        LegacyProxy& operator=(LegacyProxy&& other)      = delete;

        LegacyProxy() : proxy_value_(null_value) {}

        LegacyProxy(proxy_value_t value) : proxy_value_(value) {}

        // use to_string directly when the implicit cast is ambiguous
        operator std::string() const&&
        {
            return proxy_value_ ? *proxy_value_ : value_t::value_type {};
        }

        operator char const*() const&&
        {
            return proxy_value_ ? proxy_value_->c_str() : nullptr;
        }

        friend std::string to_string(LegacyProxy&& proxy)
        {
            return std::move(proxy).operator std::string();
        }

    private:
        static inline value_t null_value;

        proxy_value_t proxy_value_;
    };
};
