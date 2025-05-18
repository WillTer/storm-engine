#pragma once

#include <sstream>
#include <stdexcept>
#include <string>

#include <AL/al.h>

#define AL_CHECKED_CALL(call, ...) ALCheckedCall(__FILE__, __LINE__, #call, call, __VA_ARGS__)

template <typename Func, typename... Args>
auto ALCheckedCall(const std::string &filename, int line, const std::string &callname, Func f, Args... args) ->
    typename std::enable_if_t<!std::is_same_v<decltype(f(args...)), void>, decltype(f(args...))>
{
    auto ret = f(std::forward<Args>(args)...);
    if (auto err = alGetError(); err != AL_NO_ERROR)
    {
        std::stringstream ss;
        ss << "[" << filename << ":" << line << "] (" << callname << ") AL call error, code: " << std::to_string(err);
        throw std::runtime_error(ss.str());
    }

    return ret;
}

template <typename Func, typename... Args>
void ALCheckedCall(const std::string &filename, int line, const std::string &callname, Func f, Args... args)
{
    f(std::forward<Args>(args)...);
    if (auto err = alGetError(); err != AL_NO_ERROR)
    {
        std::stringstream ss;
        ss << "[" << filename << ":" << line << "] (" << callname << ") AL call error, code: " << std::to_string(err);
        throw std::runtime_error(ss.str());
    }
}
