#pragma once

#include "compiler.h"
#include "core_private.h"
#include "entity_manager.h"
#include "services_list.h"
#include "timer.h"
#include "vma.hpp"

#define ENGINE_SCRIPT_VERSION 54128

class CoreImpl final: public CorePrivate
{
public:
    void Init();

    void InitBase();
    void ReleaseBase();

    void CleanUp();

    void SetWindow(std::shared_ptr<storm::OSWindow> window) override;
    bool Initialize();
    void ResetCore();
    bool Run();
    bool LoadClassesTable();

    void ProcessExecute();
    void ProcessRealize();
    void ProcessStateLoading();
    void ProcessRunStart(uint32_t section_code);
    void ProcessRunEnd(uint32_t section_code);

    void CheckAutoExceptions(uint32_t xflag) const;
    void ReleaseServices();
    void ProcessEngineIniFile();

    void ProcessControls();

    void     DumpEntitiesInfo();
    void     EraseEntities();
    void     ClearEvents();
    void*    MakeClass(char const* class_name);
    void     AppState(bool state);
    uint32_t MakeHashValue(char const* string);
    VMA*     FindVMA(char const* class_name);
    VMA*     FindVMA(int32_t hash);
    //------------------------------------------------------------------------------------------------
    // API functions : (virtual API)

    // common programm control

    // shutdown core, delete all objects and close programm
    void Exit();
    // return application handle
    storm::OSWindow* GetWindow() override;
#ifdef _WIN32  // HINSTANCE
    HINSTANCE GetAppInstance();
#endif
    // set time scale; affect on std entity functions DeltaTime parameter
    void SetTimeScale(float _scale) override;
    // write message to system log file
    void Trace(char const* Format, ...) override;

    // return service object pointer;
    void* GetService(char const* service_name) override;

    ATTRIBUTES* Entity_GetAttributeClass(entid_t id_PTR, char const* name) override;
    char const* Entity_GetAttribute(entid_t id_PTR, char const* name) override;
    uint32_t    Entity_GetAttributeAsDword(entid_t id_PTR, char const* name, uint32_t def = 0) override;
    float       Entity_GetAttributeAsFloat(entid_t id_PTR, char const* name, float def = 0) override;
    bool        Entity_SetAttribute(entid_t id_PTR, char const* name, char const* attribute) override;
    bool        Entity_SetAttributeUseDword(entid_t id_PTR, char const* name, uint32_t val) override;
    bool        Entity_SetAttributeUseFloat(entid_t id_PTR, char const* name, float val) override;
    void        Entity_SetAttributePointer(entid_t id_PTR, ATTRIBUTES* pA) override;
    uint32_t    Entity_AttributeChanged(entid_t id_PTR, ATTRIBUTES*) override;
    ATTRIBUTES* Entity_GetAttributePointer(entid_t id_PTR) override;

    // messeges system

    // send message to an object
    uint64_t Send_Message(entid_t Destination, char const* Format, ...) override;

    // save core state
    bool SaveState(char const* file_name) override;
    // force core to load state file at the start of next game loop, return false if no state file
    bool InitiateStateLoading(char const* file_name) override;

    // return current fps
    uint32_t EngineFps() override;
    // set fixed delta time mode, (-1) - off
    void     SetDeltaTime(int32_t delta_time) override;
    uint32_t GetDeltaTime() override;
    uint32_t GetRDeltaTime() override;
    //
    VDATA*   Event(std::string_view const& event_name) override;
    VDATA*   Event(std::string_view const& event_name, MESSAGE& message) override;
    uint32_t PostEvent(char const* Event_name, uint32_t post_time, char const* Format, ...) override;

    void* GetSaveData(std::filesystem::path const& file_name, int32_t& data_size) override;

    bool SetSaveData(std::filesystem::path const& file_name, void* data_ptr, int32_t data_size) override;

    uint32_t SetScriptFunction(IFUNCINFO* pFuncInfo) override;

    void* GetScriptVariable(char const* pVariableName, uint32_t* pdwVarIndex = nullptr) override;

    [[nodiscard]] storm::ENGINE_VERSION GetTargetEngineVersion() const noexcept override;

    [[nodiscard]] ScreenSize GetScreenSize() const noexcept override;

    void stopFrameProcessing() override;

    // Entity management
    void                  AddToLayer(layer_index_t index, entid_t id, priority_t priority) override;
    void                  EraseEntity(entid_t entity) override;
    entid_t               CreateEntity(char const* name, ATTRIBUTES* attr) override;
    entptr_t              GetEntityPointer(entid_t id) const override;
    entptr_t              GetEntityPointerSafe(entid_t id) const override;
    entid_t               GetEntityId(char const* name) const override;
    bool                  IsEntityValid(entid_t id) const override;
    entity_container_cref GetEntityIds(layer_type_t type) const override;
    entity_container_cref GetEntityIds(layer_index_t index) const override;
    entity_container_cref GetEntityIds(char const* name) const override;
    void                  SetLayerType(layer_index_t index, layer_type_t type) override;
    void                  SetLayerFrozen(layer_index_t index, bool freeze) override;
    void                  RemoveFromLayer(layer_index_t index, entid_t id) override;
    hash_t                GetClassCode(entid_t id) const override;
    bool                  IsLayerFrozen(layer_index_t index) const override;
    void                  ForEachEntity(std::function<void(entptr_t)> const& f) override;

    void collectCrashInfo() const;

    [[nodiscard]] bool initialized() const
    {
        return Initialized;
    }

    TIMER Timer;

    std::unique_ptr<COMPILER> Compiler;

    bool Exit_flag;  // true if the program closing

private:
    std::unique_ptr<EntityManager> entity_manager_;

    storm::ENGINE_VERSION targetVersion_ = storm::ENGINE_VERSION::LATEST;

    bool stopFrameProcessing_ = false;

    bool                             bAppActive {};
    bool                             Memory_Leak_flag;  // true if core detected memory leak
    bool                             Root_flag;
    bool                             Initialized;  // initialized flag (false at startup or after Reset())
    bool                             bEngineIniProcessed;
    std::shared_ptr<storm::OSWindow> window_;               // application handle
    char                             gstring[MAX_PATH] {};  // general purpose string
    bool                             State_loading;
    bool                             bEnableTimeScale {};

    SERVICES_LIST Services_List;  // list for subsequent calls RunStart/RunEnd service functions

#ifdef _WIN32  // HINSTANCE
    HINSTANCE hInstance {};
#endif

    char* State_file_name;

    float fTimeScale;
};

inline CoreImpl core_internal;
