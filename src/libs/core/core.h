#pragma once

#include <functional>

#include <libs/core/v_file_service.h>
#include <libs/shared_headers/layers.h>
#include <libs/util/platform/platform.hpp>
#include <libs/window/os_window.hpp>
#include <spdlog/spdlog.h>

// common includes
#include "controls.h"
#include "engine_version.hpp"
#include "message.h"
#include "v_data.h"

struct IFUNCINFO;

struct ScreenSize {
    size_t width {};
    size_t height {};
};

// TODO: move to another place
#define STORM_DELETE(x) \
    { \
        delete x; \
        x = 0; \
    }
#define RELEASE(x) \
    { \
        if (x) x->Release(); \
        x = 0; \
    }

uint64_t get_performance_counter();
#define RDTSC_B(x) \
    { \
        x = get_performance_counter(); \
    }
#define RDTSC_E(x) \
    { \
        x = get_performance_counter() - x; \
    }
//

class Core
{
public:
    virtual ~Core() = default;

    // return application window
    virtual storm::OSWindow* GetWindow() = 0;
    // set time scale; affect on std entity functions DeltaTime parameter
    virtual void SetTimeScale(float _scale) = 0;
    // write message to system log file
    virtual void Trace(char const* Format, ...) = 0;
    // return service object pointer;
    virtual void* GetService(char const* service_name) = 0;

    virtual ATTRIBUTES* Entity_GetAttributeClass(entid_t id_PTR, char const* name)                     = 0;
    virtual char const* Entity_GetAttribute(entid_t id_PTR, char const* name)                          = 0;
    virtual uint32_t    Entity_GetAttributeAsDword(entid_t id_PTR, char const* name, uint32_t def = 0) = 0;
    virtual float       Entity_GetAttributeAsFloat(entid_t id_PTR, char const* name, float def = 0)    = 0;
    virtual bool        Entity_SetAttribute(entid_t id_PTR, char const* name, char const* attribute)   = 0;
    virtual bool        Entity_SetAttributeUseDword(entid_t id_PTR, char const* name, uint32_t val)    = 0;
    virtual bool        Entity_SetAttributeUseFloat(entid_t id_PTR, char const* name, float val)       = 0;
    virtual void        Entity_SetAttributePointer(entid_t id_PTR, ATTRIBUTES* pA)                     = 0;
    virtual uint32_t    Entity_AttributeChanged(entid_t id_PTR, ATTRIBUTES*)                           = 0;
    virtual ATTRIBUTES* Entity_GetAttributePointer(entid_t id_PTR)                                     = 0;

    // send message to an object
    virtual uint64_t Send_Message(entid_t Destination, char const* Format, ...) = 0;

    // save core state
    virtual bool SaveState(char const* file_name) = 0;
    // force core to load state file at the start of next game loop, return false if no state file
    virtual bool InitiateStateLoading(char const* file_name) = 0;

    // return current fps
    virtual uint32_t EngineFps() = 0;
    // set fixed delta time mode, (-1) - off
    virtual void     SetDeltaTime(int32_t delta_time) = 0;
    virtual uint32_t GetDeltaTime()                   = 0;
    virtual uint32_t GetRDeltaTime()                  = 0;
    //
    virtual VDATA* Event(std::string_view const& event_name) = 0;
    template <typename... Args>
    VDATA* Event(std::string_view const& event_name, std::string_view const& format, Args... args)
    {
        MESSAGE message;
        message.Reset(format, args...);
        return Event(event_name, message);
    }
    virtual VDATA*   Event(std::string_view const& event_name, MESSAGE& message)                    = 0;
    virtual uint32_t PostEvent(char const* Event_name, uint32_t post_time, char const* Format, ...) = 0;

    virtual void* GetSaveData(std::filesystem::path const& file_name, int32_t& data_size) = 0;

    virtual bool SetSaveData(std::filesystem::path const& file_name, void* data_ptr, int32_t data_size) = 0;

    virtual uint32_t SetScriptFunction(IFUNCINFO* pFuncInfo) = 0;

    virtual char const* EngineIniFileName() = 0;

    virtual void* GetScriptVariable(char const* pVariableName, uint32_t* pdwVarIndex = nullptr) = 0;

    [[nodiscard]] virtual storm::ENGINE_VERSION GetTargetEngineVersion() const noexcept = 0;

    [[nodiscard]] virtual ScreenSize GetScreenSize() const noexcept = 0;

    virtual void stopFrameProcessing() = 0;

    // Entity management
    virtual void                  AddToLayer(layer_index_t index, entid_t id, priority_t priority) = 0;
    virtual void                  EraseEntity(entid_t entity)                                      = 0;
    virtual entid_t               CreateEntity(char const* name, ATTRIBUTES* attr = nullptr)       = 0;
    virtual entptr_t              GetEntityPointer(entid_t entity) const                           = 0;
    virtual entptr_t              GetEntityPointerSafe(entid_t entity) const                       = 0;
    virtual entid_t               GetEntityId(char const* name) const                              = 0;
    virtual bool                  IsEntityValid(entid_t entity) const                              = 0;
    virtual entity_container_cref GetEntityIds(layer_type_t index) const                           = 0;
    virtual entity_container_cref GetEntityIds(layer_index_t index) const                          = 0;
    virtual entity_container_cref GetEntityIds(char const* name) const                             = 0;
    virtual void                  SetLayerType(layer_index_t index, layer_type_t type)             = 0;
    virtual void                  SetLayerFrozen(layer_index_t index, bool freeze)                 = 0;
    virtual void                  RemoveFromLayer(layer_index_t index, entid_t id)                 = 0;
    virtual hash_t                GetClassCode(entid_t id) const                                   = 0;
    virtual bool                  IsLayerFrozen(layer_index_t index) const                         = 0;
    virtual void                  ForEachEntity(std::function<void(entptr_t)> const& f)            = 0;

    CONTROLS* Controls {};
};

extern Core& core;
