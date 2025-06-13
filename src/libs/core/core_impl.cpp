#include "core_impl.h"

#include <algorithm>
#include <fstream>

#include <SDL2/SDL.h>
#include <libs/config/main_config.h>
#include <libs/filesystem/default_paths.h>
#include <libs/steam_api/steam_api.hpp>
#include <libs/util/fs.h>
#include <libs/util/string_compare.hpp>

#include "compiler.h"
#include "controls.h"

Core& core = core_internal;

uint64_t get_performance_counter()
{
    return SDL_GetPerformanceCounter();
}

uint32_t dwNumberScriptCommandsExecuted = 0;

typedef struct {
    uint32_t code;
    void*    pointer;
} CODE_AND_POINTER;

void CoreImpl::ResetCore()
{
    Initialized         = false;
    bEngineIniProcessed = false;

    ReleaseServices();

    Services_List.Release();

    STORM_DELETE(State_file_name);
}

void CoreImpl::CleanUp()
{
    Initialized         = false;
    bEngineIniProcessed = false;
    ReleaseServices();
    Compiler->Release();
    Services_List.Release();
    Services_List.Release();
    delete[] State_file_name;
}

void CoreImpl::SetWindow(std::shared_ptr<storm::OSWindow> window)
{
    window_ = std::move(window);
}

void CoreImpl::Init(std::shared_ptr<storm::ServiceLocator> const& service_locator)
{
    Initialized         = false;
    bEngineIniProcessed = false;
    State_file_name     = nullptr;
    Exit_flag           = false;
    State_loading       = false;
    Memory_Leak_flag    = false;
    Controls            = nullptr;
    fTimeScale          = 1.0f;
    Compiler            = std::make_unique<COMPILER>(service_locator);
    entity_manager_     = std::make_unique<EntityManager>(service_locator);
    m_service_locator   = service_locator;

    /* TODO: place this outside CoreImpl */
    SetLayerType(EXECUTE, layer_type_t::execute);
    SetLayerType(REALIZE, layer_type_t::realize);
    SetLayerType(SEA_EXECUTE, layer_type_t::execute);
    SetLayerType(SEA_REALIZE, layer_type_t::realize);
    SetLayerType(INTERFACE_EXECUTE, layer_type_t::execute);
    SetLayerType(INTERFACE_REALIZE, layer_type_t::realize);
    SetLayerType(FADER_EXECUTE, layer_type_t::execute);
    SetLayerType(FADER_REALIZE, layer_type_t::realize);
    SetLayerType(LIGHTER_EXECUTE, layer_type_t::execute);
    SetLayerType(LIGHTER_REALIZE, layer_type_t::realize);
    SetLayerType(VIDEO_EXECUTE, layer_type_t::execute);
    SetLayerType(VIDEO_REALIZE, layer_type_t::realize);
    SetLayerType(EDITOR_REALIZE, layer_type_t::realize);
    SetLayerType(INFO_REALIZE, layer_type_t::realize);
    SetLayerType(SOUND_DEBUG_REALIZE, layer_type_t::realize);
}

void CoreImpl::InitBase()
{
    LoadClassesTable();
}

void CoreImpl::ReleaseBase()
{
    Compiler->Token.Release();
}

bool CoreImpl::Run()
{
    stopFrameProcessing_ = false;

    auto const bDebugWindow = true;
    if (bDebugWindow && core_internal.Controls && core_internal.Controls->GetDebugAsyncKeyState(VK_F7) < 0) DumpEntitiesInfo();
    dwNumberScriptCommandsExecuted = 0;

    if (Exit_flag) return false;  // exit

    Timer.Run();  // calc delta time

    auto* pVCTime = static_cast<VDATA*>(core_internal.GetScriptVariable("iRealDeltaTime"));
    if (pVCTime) pVCTime->Set(static_cast<int32_t>(GetRDeltaTime()));

    auto tt       = std::time(nullptr);
    auto local_tm = *std::localtime(&tt);

    auto* pVYear  = static_cast<VDATA*>(core_internal.GetScriptVariable("iRealYear"));
    auto* pVMonth = static_cast<VDATA*>(core_internal.GetScriptVariable("iRealMonth"));
    auto* pVDay   = static_cast<VDATA*>(core_internal.GetScriptVariable("iRealDay"));

    if (pVYear) pVYear->Set(local_tm.tm_year + 1900);
    if (pVMonth) pVMonth->Set(local_tm.tm_mon + 1);  // tm_mon belongs [0, 11]
    if (pVDay) pVDay->Set(local_tm.tm_mday);

    if (Controls && Controls->GetDebugAsyncKeyState('R') < 0) Timer.Delta_Time *= 10;
    if (Controls && Controls->GetDebugAsyncKeyState('Y') < 0) Timer.Delta_Time = static_cast<uint32_t>(Timer.Delta_Time * 0.2f);

    Timer.Delta_Time = static_cast<uint32_t>(Timer.Delta_Time * fTimeScale);
    Timer.fDeltaTime *= fTimeScale;

    auto* pVData = static_cast<VDATA*>(GetScriptVariable("fHighPrecisionDeltaTime", nullptr));
    if (pVData) pVData->Set(Timer.fDeltaTime * 0.001f);

    if (!Initialized) {
        Initialize();  // initialization at start or after reset
    }
    if (!bEngineIniProcessed) ProcessEngineIniFile();

    Compiler->ProcessFrame(Timer.GetDeltaTime());
    Compiler->ProcessEvent("frame");

    ProcessStateLoading();

    ProcessRunStart(SECTION_ALL);
    if (stopFrameProcessing_) {
        // service asked to skip current frame processing
        return true;
    }

    ProcessExecute();  // transfer control to objects via Execute() function
    ProcessRealize();  // transfer control to objects via Realize() function

    steamapi::SteamApi::getInstance().RunCallbacks();

    if (Controls) Controls->Update(Timer.rDelta_Time);

    if (Controls) ProcessControls();

    entity_manager_->NewLifecycle();

    ProcessRunEnd(SECTION_ALL);

    return true;
}

void CoreImpl::ProcessControls()
{
    CONTROL_STATE cs;
    USER_CONTROL  uc;

    if (!Controls) return;

    for (int32_t n = 0; n < Controls->GetControlsNum(); n++) {
        Controls->GetControlState(n, cs);
        if (cs.state == CST_ACTIVATED) {
            Controls->GetControlDesc(n, uc);
            Core::Event("Control Activation", "s", uc.name);
        } else if (cs.state == CST_INACTIVATED) {
            Controls->GetControlDesc(n, uc);
            Core::Event("Control Deactivation", "s", uc.name);
        }
    }
}

//-------------------------------------------------------------------------------------------------
// internal functions
//-------------------------------------------------------------------------------------------------
bool CoreImpl::Initialize()
{
    ResetCore();

    Initialized = true;

    return true;
}

void CoreImpl::ProcessEngineIniFile()
{
    bEngineIniProcessed = true;

    auto const& config_loader = m_service_locator->get<storm::IConfigLoader>();
    auto const  script_info   = storm::main_config::script_info(*config_loader);
    auto const  controls_info = storm::main_config::controls_info(*config_loader);

    auto const program_dir = fio->base_directory_path(BaseDirectory::Program);
    Compiler->SetProgramDirectory(program_dir.string().c_str());

    if (!controls_info.scheme.empty()) {
        core_internal.Controls = static_cast<CONTROLS*>(MakeClass(controls_info.scheme.c_str()));
        if (core_internal.Controls == nullptr) { core_internal.Controls = static_cast<CONTROLS*>(MakeClass("controls")); }
    } else {
        delete Controls;
        Controls = nullptr;

        core_internal.Controls = new CONTROLS;
    }

    core_internal.Controls->Init(m_service_locator);

    auto const compat_info = storm::main_config::compatibility_info(*config_loader);
    targetVersion_         = compat_info.target_version;

    if (!Compiler->CreateProgram(script_info.entry_point.c_str())) { throw std::runtime_error("fail to create program"); }
    if (!Compiler->Run()) { throw std::runtime_error("fail to run program"); }

    // Script version test
    if (targetVersion_ >= storm::ENGINE_VERSION::LATEST) {
        auto  script_version      = std::numeric_limits<int32_t>::max();
        auto* script_version_data = static_cast<VDATA*>(core_internal.GetScriptVariable("iScriptVersion"));
        if (script_version_data != nullptr) { script_version_data->Get(script_version); }

        if (script_version != ENGINE_SCRIPT_VERSION) {
#ifdef _WIN32  // FIX_LINUX Cursor
            ShowCursor(SDL_TRUE);
#endif
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Wrong script version", nullptr);
            Compiler->ExitProgram();
        }
    }
}

bool CoreImpl::LoadClassesTable()
{
    for (auto* c: __STORM_CLASSES_REGISTRY) {
        auto const hash = MakeHashValue(c->GetName());
        c->SetHash(hash);
    }

    return true;
}

void CoreImpl::CheckAutoExceptions(uint32_t = 0) const
{
    spdlog::warn("exception thrown");
}

void CoreImpl::Exit()
{
    Exit_flag = true;
}

storm::OSWindow* CoreImpl::GetWindow()
{
    return window_.get();
}

#ifdef _WIN32  // HINSTANCE
HINSTANCE CoreImpl::GetAppInstance()
{
    return hInstance;
}
#endif

void CoreImpl::SetTimeScale(float _scale)
{
    fTimeScale = _scale;
}

//------------------------------------------------------------------------------------------------
// transfer message arguments and program control to entity, specified by Destination id
//
uint64_t CoreImpl::Send_Message(entid_t Destination, char const* Format, ...)
{
    MESSAGE     message;
    auto* const ptr = GetEntityPointerSafe(Destination);  // check for valid destination
    if (!ptr) return 0;

    va_list args;
    va_start(args, Format);
    message.ResetVA(Format, args);
    auto const rc = static_cast<Entity*>(ptr)->ProcessMessage(message);  // transfer control
    va_end(args);
    return rc;
}

uint32_t CoreImpl::PostEvent(char const* Event_name, uint32_t post_time, char const* Format, ...)
{
    MESSAGE* pMS;
    MESSAGE  message;

    entid_t id;

    if (Format != nullptr) {
        pMS = new MESSAGE();
        va_list args;
        va_start(args, Format);
        message.ResetVA(Format, args);
        pMS->Reset(Format);

        auto bAction = true;
        while (bAction) {
            switch (message.GetCurrentFormatType()) {
                //-------------------------------------
            case 'l':
                int32_t v;
                v = message.Long();
                pMS->Set(v);
                break;
            case 'f':
                float f;
                f = message.Float();
                pMS->Set(f);
                break;
            case 'i':
                id = message.EntityID();
                pMS->SetEntity(id);
                break;
            case 'e':
                VDATA* e;
                e = message.ScriptVariablePointer();
                pMS->Set(e);
                break;
            case 's': {
                std::string const& s = message.StringPointer();
                pMS->Set(s);
                break;
            }
            case 'a':
                ATTRIBUTES* a;
                a = message.AttributePointer();
                pMS->Set(a);
                break;

                //-------------------------------------
            default: bAction = false; break;
            }
        }
        va_end(args);
    } else
        pMS = nullptr;

    auto* pEM     = new S_EVENTMSG(Event_name, pMS, post_time);
    pEM->bProcess = true;
    Compiler->AddPostEvent(pEM);
    return 0;
}

VDATA* CoreImpl::Event(std::string_view const& event_name)
{
    MESSAGE message;
    return Compiler->ProcessEvent(event_name.data(), message);
}

VDATA* CoreImpl::Event(std::string_view const& event_name, MESSAGE& message)
{
    return Compiler->ProcessEvent(event_name.data(), message);
}

void* CoreImpl::MakeClass(char const* class_name)
{
    int32_t const hash = MakeHashValue(class_name);
    for (auto* const c: __STORM_CLASSES_REGISTRY)
        if (c->GetHash() == hash && storm::iEquals(class_name, c->GetName())) return c->CreateClass();

    return nullptr;
}

void CoreImpl::ReleaseServices()
{
    for (auto* const c: __STORM_CLASSES_REGISTRY)
        if (c->Service()) c->Clear();

    Controls = nullptr;
}

VMA* CoreImpl::FindVMA(char const* class_name)
{
    int32_t const hash = MakeHashValue(class_name);
    for (auto* const c: __STORM_CLASSES_REGISTRY)
        if (c->GetHash() == hash && storm::iEquals(class_name, c->GetName())) return c;

    return nullptr;
}

VMA* CoreImpl::FindVMA(int32_t hash)
{
    for (auto* const c: __STORM_CLASSES_REGISTRY)
        if (c->GetHash() == hash) return c;

    return nullptr;
}

void* CoreImpl::GetService(char const* service_name)
{
    auto* pClass = FindVMA(service_name);
    if (pClass == nullptr) {
        CheckAutoExceptions(0);
        return nullptr;
    }

    if (pClass->GetHash() == 0) {
        CheckAutoExceptions(0);
        return nullptr;
    }

    if (pClass->GetReference() > 0) return pClass->CreateClass();

    auto* service_PTR = static_cast<SERVICE*>(pClass->CreateClass());

    auto const class_code = MakeHashValue(service_name);
    pClass->SetHash(class_code);

    if (!service_PTR->Init(m_service_locator)) {
        CheckAutoExceptions(0);
        return nullptr;
    }

    Services_List.Add(class_code, class_code, service_PTR);

    return service_PTR;
}

void CoreImpl::Trace(char const* format, ...)
{
    static char buffer_4k[4096];

    va_list args;
    va_start(args, format);
    vsnprintf(buffer_4k, sizeof(buffer_4k) - 4, format, args);
    va_end(args);
    spdlog::info(buffer_4k);
}

//------------------------------------------------------------------------------------------------
// Transfer programm control to objects via Execute() functions
//
void CoreImpl::ProcessExecute()
{
    ProcessRunStart(SECTION_EXECUTE);

    auto const  deltatime = Timer.GetDeltaTime();
    auto const& entIds    = core.GetEntityIds(layer_type_t::execute);
    for (auto id: entIds) {
        if (auto* ptr = core.GetEntityPointerSafe(id)) { ptr->ProcessStage(Entity::Stage::execute, deltatime); }
    }

    ProcessRunEnd(SECTION_EXECUTE);
}

void CoreImpl::ProcessRealize()
{
    ProcessRunStart(SECTION_REALIZE);

    auto const  deltatime = Timer.GetDeltaTime();
    auto const& entIds    = core.GetEntityIds(layer_type_t::realize);
    for (auto id: entIds) {
        if (auto* ptr = core.GetEntityPointerSafe(id)) { ptr->ProcessStage(Entity::Stage::realize, deltatime); }
    }

    ProcessRunEnd(SECTION_REALIZE);
}

// save core state
bool CoreImpl::SaveState(char const* file_name)
{
    if (!file_name) { throw std::logic_error("Bad file name of save"); }

    auto fileS = fio->open_file<std::ofstream>(file_name, std::ios::binary);

    if (!fileS.is_open()) { return false; }

    Compiler->SaveState(fileS);

    return true;
}

// force core to load state file at the start of next game loop, return false if no state file
bool CoreImpl::InitiateStateLoading(char const* file_name)
{
    if (!fio->exists(file_name)) { return false; }
    delete[] State_file_name;

    auto const len  = strlen(file_name) + 1;
    State_file_name = static_cast<char*>(new char[len]);
    strcpy_s(State_file_name, len, file_name);
    return true;
}

void CoreImpl::ProcessStateLoading()
{
    if (!State_file_name) { return; }

    State_loading = true;
    EraseEntities();

    auto fileS = fio->open_file<std::ifstream>(State_file_name, std::ios::binary);
    if (!fileS.is_open()) { return; }
    Compiler->LoadState(fileS);

    delete[] State_file_name;
    State_file_name = nullptr;
    State_loading   = false;
}

void CoreImpl::ProcessRunStart(uint32_t section_code)
{
    uint32_t class_code;
    SERVICE* service_PTR = Services_List.GetService(class_code);
    while (service_PTR) {
        uint32_t const section = service_PTR->RunSection();
        if (section == section_code) { service_PTR->RunStart(); }
        service_PTR = Services_List.GetServiceNext(class_code);
    }

    for (auto const& service: m_registered_services) {
        if (auto const service_shared = service.lock(); service_shared && service_shared->RunSection() == section_code) {
            service_shared->RunStart();
        }
    }
}

void CoreImpl::ProcessRunEnd(uint32_t section_code)
{
    uint32_t class_code;
    SERVICE* service_PTR = Services_List.GetService(class_code);
    while (service_PTR) {
        uint32_t const section = service_PTR->RunSection();
        if (section == section_code) { service_PTR->RunEnd(); }
        service_PTR = Services_List.GetServiceNext(class_code);
    }

    for (auto const& service: m_registered_services) {
        if (auto const service_shared = service.lock(); service_shared && service_shared->RunSection() == section_code) {
            service_shared->RunEnd();
        }
    }
}

uint32_t CoreImpl::EngineFps()
{
    return Timer.fps;
}

void CoreImpl::SetDeltaTime(int32_t delta_time)
{
    Timer.SetDelta(delta_time);
}

uint32_t CoreImpl::GetDeltaTime()
{
    return Timer.GetDeltaTime();
}

uint32_t CoreImpl::GetRDeltaTime()
{
    return Timer.rDelta_Time;
}

ATTRIBUTES* CoreImpl::Entity_GetAttributeClass(entid_t id_PTR, char const* name)
{
    Entity* pE = GetEntityPointer(id_PTR);
    if (pE == nullptr) return nullptr;
    if (pE->AttributesPointer == nullptr) return nullptr;
    return pE->AttributesPointer->FindAClass(pE->AttributesPointer, name);
}

char const* CoreImpl::Entity_GetAttribute(entid_t id_PTR, char const* name)
{
    Entity* pE = GetEntityPointer(id_PTR);
    if (pE == nullptr) return nullptr;
    if (pE->AttributesPointer == nullptr) return nullptr;
    return pE->AttributesPointer->GetAttribute(name);
}

uint32_t CoreImpl::Entity_GetAttributeAsDword(entid_t id_PTR, char const* name, uint32_t def)
{
    Entity* pE = GetEntityPointer(id_PTR);
    if (pE == nullptr) return def;
    if (pE->AttributesPointer == nullptr) return def;
    return pE->AttributesPointer->GetAttributeAsDword(name, def);
}

float CoreImpl::Entity_GetAttributeAsFloat(entid_t id_PTR, char const* name, float def)
{
    Entity* pE = GetEntityPointer(id_PTR);
    if (pE == nullptr) return def;
    if (pE->AttributesPointer == nullptr) return def;
    return pE->AttributesPointer->GetAttributeAsFloat(name, def);
}

bool CoreImpl::Entity_SetAttribute(entid_t id_PTR, char const* name, char const* attribute)
{
    Entity* pE = GetEntityPointer(id_PTR);
    if (pE == nullptr) return false;
    if (pE->AttributesPointer == nullptr) return false;
    return pE->AttributesPointer->SetAttribute(name, attribute);
}

bool CoreImpl::Entity_SetAttributeUseDword(entid_t id_PTR, char const* name, uint32_t val)
{
    Entity* pE = GetEntityPointer(id_PTR);
    if (pE == nullptr) return false;
    if (pE->AttributesPointer == nullptr) return false;
    return pE->AttributesPointer->SetAttributeUseDword(name, val);
}

bool CoreImpl::Entity_SetAttributeUseFloat(entid_t id_PTR, char const* name, float val)
{
    Entity* pE = GetEntityPointer(id_PTR);
    if (pE == nullptr) return false;
    if (pE->AttributesPointer == nullptr) return false;
    return pE->AttributesPointer->SetAttributeUseFloat(name, val);
}

void CoreImpl::Entity_SetAttributePointer(entid_t id_PTR, ATTRIBUTES* pA)
{
    Entity* pE = GetEntityPointer(id_PTR);
    if (pE == nullptr) return;
    pE->AttributesPointer = pA;
}

uint32_t CoreImpl::Entity_AttributeChanged(entid_t id_PTR, ATTRIBUTES* pA)
{
    Entity* pE = GetEntityPointer(id_PTR);
    if (pE == nullptr) return 0;
    return pE->AttributeChanged(pA);
}

ATTRIBUTES* CoreImpl::Entity_GetAttributePointer(entid_t id_PTR)
{
    Entity* pE = GetEntityPointer(id_PTR);
    if (pE == nullptr) return nullptr;
    return pE->AttributesPointer;
}

void CoreImpl::EraseEntities()
{
    entity_manager_->EraseAll();
}

void CoreImpl::ClearEvents()
{
    Compiler->ClearEvents();
}

void CoreImpl::AppState(bool state)
{
    if (Controls) Controls->AppState(state);
}

uint32_t CoreImpl::MakeHashValue(char const* string)
{
    uint32_t hval = 0;

    while (*string != 0) {
        char v = *string++;
        if ('A' <= v && v <= 'Z') v += 'a' - 'A';

        hval             = (hval << 4) + static_cast<uint32_t>(v);
        uint32_t const g = hval & (static_cast<uint32_t>(0xf) << (32 - 4));
        if (g != 0) {
            hval ^= g >> (32 - 8);
            hval ^= g;
        }
    }
    return hval;
}

//==========================================================================================================================
// end
//==========================================================================================================================

void CoreImpl::DumpEntitiesInfo()
{
    /*LARGE_INTEGER li;
    if (!QueryPerformanceFrequency(&li))
      throw std::runtime_error();

    const auto freq = double(li.QuadPart)/1000.0;

    uint32_t n;
    char * ptr;
    VMA * pClass;

    Trace("Script commands executed: %" PRIu32, dwNumberScriptCommandsExecuted);
    Trace("Entity Dump -----------------------------------");

    for(n=0;n<=CoreState.Atoms_max_orbit;n++)
    {
      if(Atoms_PTR[n] == nullptr) continue;

      pClass = FindVMA(Atoms_PTR[n]->atom_id.class_code);
      if(pClass)
      {
        ptr = pClass->GetName();
        if(ptr)
        {
          Trace("Class: %s", ptr);
          Trace("     : Realize:  Cur( %fms ) Max( %fms )", Atoms_PTR[n]->as.Realize_ticks_av/freq,
    Atoms_PTR[n]->as.Realize_ticks_max/freq); Trace("     : Execute:  Cur( %fms ) Max( %fms )",
    Atoms_PTR[n]->as.Execute_ticks_av/freq, Atoms_PTR[n]->as.Execute_ticks_max/freq);
        }
      }
    }

    Trace(" ------- Running objects ------- ");
    for(n=0;n<=CommonLayers.lss.Layer_max_index;n++)
    {
      LAYER* l_PTR = CommonLayers.Layer_Table[n];
      if(l_PTR == nullptr) continue;

      uint32_t flags = l_PTR->ls.Flags & LRFLAG_FROZEN;
      if(flags != 0)
      {
        Trace("LAYER: %s frozen -------------------------------------",l_PTR->Name);
        continue;
      }
      Trace("LAYER: %s --------------------------------------",l_PTR->Name);
      entid_t  eid_PTR = l_PTR->GetId();
      while(eid_PTR)
      {
        Trace("Entity:");
        if(ValidateEntity(eid_PTR))
        {
          pClass = FindVMA(Atoms_PTR[eid_PTR->atom_position]->atom_id.class_code);
          if(pClass) ptr = pClass->GetName();
          Trace("        %s",ptr);
          if(Atoms_PTR[eid_PTR->atom_position]->as.Deleted) Trace("        deleted");

        }
        eid_PTR = l_PTR->GetNextID();
      }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));*/
}

void* CoreImpl::GetSaveData(std::filesystem::path const& file_name, int32_t& data_size)
{
    return Compiler->GetSaveData(file_name, data_size);
}

bool CoreImpl::SetSaveData(std::filesystem::path const& file_name, void* data_ptr, int32_t data_size)
{
    return Compiler->SetSaveData(file_name, data_ptr, data_size);
}

uint32_t CoreImpl::SetScriptFunction(IFUNCINFO* pFuncInfo)
{
    return Compiler->SetScriptFunction(pFuncInfo);
}

void* CoreImpl::GetScriptVariable(char const* pVariableName, uint32_t* pdwVarIndex)
{
    VarInfo const* real_var;

    auto const dwVarIndex = Compiler->VarTab.FindVar(pVariableName);
    if (dwVarIndex == INVALID_VAR_CODE) { return nullptr; }

    real_var = Compiler->VarTab.GetVar(dwVarIndex);
    if (real_var == nullptr) { return nullptr; }

    if (pdwVarIndex) *pdwVarIndex = dwVarIndex;

    return real_var->value.get();
}

storm::ENGINE_VERSION CoreImpl::GetTargetEngineVersion() const noexcept
{
    return targetVersion_;
}

ScreenSize CoreImpl::GetScreenSize() const noexcept
{
    switch (targetVersion_) {
    case storm::ENGINE_VERSION::PIRATES_OF_THE_CARIBBEAN: {
        return {640, 480};
    }
    default: {
        return {800, 600};
    }
    }
}

void CoreImpl::stopFrameProcessing()
{
    stopFrameProcessing_ = true;
}

void CoreImpl::AddToLayer(layer_index_t index, entid_t id, priority_t priority)
{
    entity_manager_->AddToLayer(index, id, priority);
}

void CoreImpl::EraseEntity(entid_t entity)
{
    entity_manager_->EraseEntity(entity);
}

entid_t CoreImpl::CreateEntity(char const* name, ATTRIBUTES* attr)
{
    return entity_manager_->CreateEntity(name, attr);
}

entptr_t CoreImpl::GetEntityPointer(entid_t id) const
{
    return entity_manager_->GetEntityPointer(id);
}

entptr_t CoreImpl::GetEntityPointerSafe(entid_t id) const
{
    return entity_manager_->IsEntityValid(id) ? GetEntityPointer(id) : nullptr;
}

entid_t CoreImpl::GetEntityId(char const* name) const
{
    return entity_manager_->GetEntityId(name);
}

bool CoreImpl::IsEntityValid(entid_t id) const
{
    return entity_manager_->IsEntityValid(id);
}

entity_container_cref CoreImpl::GetEntityIds(layer_type_t type) const
{
    return entity_manager_->GetEntityIds(type);
}

entity_container_cref CoreImpl::GetEntityIds(layer_index_t index) const
{
    return entity_manager_->GetEntityIds(index);
}

entity_container_cref CoreImpl::GetEntityIds(char const* name) const
{
    return entity_manager_->GetEntityIds(name);
}

void CoreImpl::SetLayerType(layer_index_t index, layer_type_t type)
{
    entity_manager_->SetLayerType(index, type);
}

void CoreImpl::SetLayerFrozen(layer_index_t index, bool freeze)
{
    entity_manager_->SetLayerFrozen(index, freeze);
}

void CoreImpl::RemoveFromLayer(layer_index_t index, entid_t id)
{
    entity_manager_->RemoveFromLayer(index, id);
}

hash_t CoreImpl::GetClassCode(entid_t id) const
{
    return entity_manager_->GetClassCode(id);
}

bool CoreImpl::IsLayerFrozen(layer_index_t index) const
{
    return entity_manager_->IsLayerFrozen(index);
}

void CoreImpl::ForEachEntity(std::function<void(entptr_t)> const& f)
{
    entity_manager_->ForEachEntity(f);
}

void CoreImpl::register_service(std::weak_ptr<SERVICE> const& service)
{
    auto const service_shared = service.lock();
    if (!service_shared) { return; }

    if (std::any_of(m_registered_services.begin(), m_registered_services.end(), [service_shared](auto const& entry) {
            return entry.lock() && entry.lock() == service_shared;
        })) {
        return;
    }

    m_registered_services.push_back(service);
}

void CoreImpl::unregister_service(std::weak_ptr<SERVICE> const& service)
{
    auto const service_shared = service.lock();
    m_registered_services.erase(
        std::remove_if(
            m_registered_services.begin(),
            m_registered_services.end(),
            [service_shared](auto const& entry) { return !entry.lock() || entry.lock() == service_shared; }),
        m_registered_services.end());
}

void CoreImpl::collectCrashInfo() const
{
    Compiler->CollectCallStack();
}
