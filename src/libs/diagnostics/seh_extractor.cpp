#ifdef WIN32
#include "seh_extractor.hpp"

#include <exception>
#include <format>

namespace
{

class [[nodiscard]] va_to_any_ptr
{
public:
    va_to_any_ptr(nullptr_t const) {}
    va_to_any_ptr(uintptr_t const ptr) : ptr_(ptr) {}
    va_to_any_ptr(void const* ptr) : ptr_(reinterpret_cast<uintptr_t>(ptr)) {}

    template <typename T>
    operator T()
    {
        return T(ptr_);
    }

private:
    uintptr_t const ptr_ {};
};

template <typename F>
[[nodiscard]] va_to_any_ptr get_rtti(ThrowInfo* throw_info, F&& get_va)
{
    CatchableTypeArray const* ctype_array = get_va(throw_info->pCatchableTypeArray);
    if (ctype_array) {
        CatchableType const* ctype = get_va(ctype_array->arrayOfCatchableTypes[0]);
        if (ctype) { return get_va(ctype->pType); }
    }
    return nullptr;
}

[[nodiscard]] int check_magic(auto magic)
{
    switch (magic) {
    case EH_MAGIC_NUMBER1: return 1;
    case EH_MAGIC_NUMBER2: return 2;
    case EH_MAGIC_NUMBER3: return 3;
    case EH_PURE_MAGIC_NUMBER1: return 4;
    default: return -1;
    }
}

}  // namespace
seh_extractor::seh_extractor(const EXCEPTION_POINTERS* ep) : ep_(ep)
{
    if (ep_ == nullptr || !ep_->ExceptionRecord) { throw std::runtime_error(""); }

    code_ = ep_->ExceptionRecord->ExceptionCode;
}

bool seh_extractor::is_abnormal() const
{
    // the following magic comes from
    // https://docs.microsoft.com/en-us/archive/blogs/stevejs/naming-threads-in-win32-and-net
    constexpr auto set_thread_description_exc = 0x406D1388;
    return code_ != DBG_PRINTEXCEPTION_C && code_ != set_thread_description_exc;
}

void seh_extractor::sink(sink_func f) const
{
    sink(f, nullptr);
}

void seh_extractor::sink(sink_func const f, EXCEPTION_RECORD* next) const
{
    // reduce stack usage
    static char buf[1024];

    if (!next) {
        f("(root exception)");
    } else {
        f("(nested exception)");
    }

    auto const* record = next ? next : ep_->ExceptionRecord;
    auto const* params = record->ExceptionInformation;

    auto const magic      = params[0];
    auto const exc_obj    = record->ExceptionInformation[1];
    auto const throw_info = record->ExceptionInformation[2];

#ifdef _WIN64
    const auto params_count = record->NumberParameters;
    auto       get_va       = [module_offset = params_count >= 4 ? record->ExceptionInformation[3] : 0](auto const rva) -> va_to_any_ptr {
        return module_offset + rva;
    };
#else
    auto get_va = [](const auto va) { return va; };
#endif

    // sink error code message
    if (record->ExceptionCode) {
        auto const error_message = FormatMessageA(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE,
            LoadLibrary(L"ntdll"),
            code_,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            buf,
            std::size(buf),
            nullptr);
        if (error_message > 0) { f(buf); }
    }

    // sink exc magic
    auto const [_, size] = std::format_to_n(buf, std::size(buf), "magic={:#x} (classified {})", magic, check_magic(magic));
    buf[size]            = '\0';
    f(buf);

    // sink exc rtti name
    if (throw_info) {
        if (std::type_info const* info = get_rtti(reinterpret_cast<ThrowInfo const*>(throw_info), std::move(get_va))) { f(info->name()); }
    }

    // sink exc data
    // TODO: may be extended with engine-specific exception data
    if (exc_obj) {
        if (auto const exc = reinterpret_cast<std::exception const*>(exc_obj)) {
            __try {
                f(exc->what());
            } __except (EXCEPTION_EXECUTE_HANDLER) {
            }
        }
    }

    // chaining
    if (record->ExceptionRecord) { sink(f, record->ExceptionRecord); }
}
#endif
