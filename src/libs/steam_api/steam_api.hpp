#pragma once

#include <cstdint>
#include <memory>

namespace steamapi
{
class SteamApi;

namespace detail
{
std::unique_ptr<SteamApi> factory(bool);
}  // namespace detail

class SteamApi
{
public:
    static SteamApi& getInstance(bool const mock = true)
    {
        static std::unique_ptr<SteamApi> steam_api = detail::factory(mock);
        return *steam_api;
    }

    virtual ~SteamApi() = default;

    virtual bool isSteamEnabled()
    {
        return false;
    }

    virtual void InitAchievements() {}

    virtual void DeleteAchievements() {}

    virtual uint32_t SetAchievementState(char const* ID)
    {
        return 0;
    }

    virtual uint32_t GetAchievementState(char const* ID)
    {
        return 0;
    }

    virtual uint32_t SetStatValue(char const* ID, uint32_t Value)
    {
        return 0;
    }

    virtual uint32_t GetStatValue(char const* ID)
    {
        return 0;
    }

    virtual uint32_t StoreStats()
    {
        return 0;
    }

    virtual bool isSteamConnected()
    {
        return false;
    }

    virtual bool ResetStats(bool bAchievementsToo)
    {
        return 0;
    }

    virtual bool ClearAchievement(char const* ID)
    {
        return 0;
    }

    virtual void InitSteamDLC() {}

    virtual void DeleteSteamDLC() {}

    virtual bool isDLCActive(uint32_t nDLC)
    {
        return true;
    }

    virtual uint32_t getDLCCount()
    {
        return 0;
    }

    virtual uint32_t getDLCDataByIndex(uint32_t iDLC)
    {
        return 0;
    }

    virtual bool activateGameOverlayDLC(uint32_t nAppId)
    {
        return false;
    }

    virtual void RunCallbacks() {}
};
}  // namespace steamapi
