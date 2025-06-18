#pragma once

#include <libs/battle_interface/bi_defines.h>

class BIImageRender;
class BI_ManagerBase;

class BI_MousePointerBase
{
public:
    BI_MousePointerBase()          = default;
    virtual ~BI_MousePointerBase() = default;

    virtual void Update() = 0;
};

class BI_ManagerNodeBase
{
public:
    BI_ManagerNodeBase() {}

    virtual ~BI_ManagerNodeBase() {}

    virtual void Update() = 0;

    virtual int32_t Event(char const* pcEventName) = 0;

    virtual bool IsPointInside(float fX, float fY) = 0;
    virtual bool IsMouseClickReaction()            = 0;
    virtual bool IsMouseMoveReaction()             = 0;
};

class BI_ManagerBase: public Entity
{
public:
    BI_ManagerBase() {}

    ~BI_ManagerBase() override {}

    // VDX9RENDER* GetDX9Render() const
    // {
    //     return m_pRS;
    // }

    BIImageRender* GetImageRender() const
    {
        return m_pImgRender;
    }

    BI_MousePointerBase* GetMouse() const
    {
        return m_pMouse;
    }

    virtual BI_ManagerNodeBase*
    CreateImageNode(char const* texture, storm::FRect const& uv, storm::Rect const& pos, uint32_t color, int32_t nPrioritet) = 0;
    virtual BI_ManagerNodeBase* CreateStringNode(
        char const*        text,
        char const*        font,
        uint32_t           color,
        float              scale,
        storm::Rect const& pos,
        int32_t            nHAlign,
        int32_t            nVAlign,
        int32_t            prioritet)                            = 0;
    virtual void DeleteNode(BI_ManagerNodeBase* pNod) = 0;

protected:
    // VDX9RENDER*          m_pRS;
    BIImageRender*       m_pImgRender;
    BI_MousePointerBase* m_pMouse;
};
