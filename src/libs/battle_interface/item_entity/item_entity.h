#pragma once

#include <string>

#include <libs/animation/animation.h>
#include <libs/battle_interface/bi_defines.h>
#include <libs/particles/v_particle_system.h>

class MODEL;
class NODE;

class ItemEntity: public Entity
{
public:
    ItemEntity();
    ~ItemEntity() override;

    bool     Init() override;
    void     Realize(uint32_t delta_time);
    uint64_t ProcessMessage(MESSAGE& message) override;

    void ProcessStage(Stage stage, uint32_t delta) override
    {
        switch (stage) {
            // case Stage::execute:
            //    Execute(delta); break;
        case Stage::realize:
            Realize(delta);
            break;
            /*case Stage::lost_render:
              LostRender(delta); break;
            case Stage::restore_render:
              RestoreRender(delta); break;*/
        }
    }

protected:
    bool ReadAndCreate();
    void SetBeginData();
    void Release();

    void SetModelToPosition(CMatrix const& mtx) const;
    void SetTechnique(char const* pcTechnique) const;
    bool TieToLocator(entid_t mdlEID, char const* pcLocName);
    void UnTieFromLocator();
    void EndEventProcess();

    void DrawIntoLocator();

    void    SetEventListener(entid_t mdlEID, entid_t mdlToTieEID, char const* pcLocName, char const* pcStartEvent, char const* pcEndEvent);
    entid_t GetModelEIDFromCharacterEID(entid_t chrEID);

protected:  // data
    bool    m_bVisible;
    entid_t m_eidModel;
    MODEL*  m_pModel;
    CMatrix m_mtxpos;

    bool        m_bTieToLocator;
    entid_t     m_eidTieModel;
    NODE*       m_pMdlNode;
    std::string m_sTieLocName;

    class EventListener: public AnimationEventListener
    {
    public:
        // Accept event
        void Event(Animation* animation, int32_t playerIndex, char const* eventName) override;

        ItemEntity* item;
        std::string m_sStartEvent;
        std::string m_sEndEvent;
        bool        m_bStartWaiting;
        entid_t     m_eidListenedModel;
        entid_t     m_eidToTieModel;
        std::string m_sToTieLocator;
    };

    friend EventListener;
    EventListener m_eventListener;

    VPARTICLE_SYSTEM* m_pParticle;
    bool              CreateParticle();
    void              DeleteParticle();
};
