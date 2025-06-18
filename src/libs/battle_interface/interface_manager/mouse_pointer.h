#pragma once

#include <string>

#include "base_manager.h"

#define BI_CURSORS_QUANTITY 10
#define BI_CURSOR_COMMON 0

class IBIImage;

class MousePointer: public BI_MousePointerBase
{
public:
    MousePointer(BI_ManagerBase* pManager, ATTRIBUTES* pARoot);
    ~MousePointer() override;

    void Update() override;

protected:
    BI_ManagerBase* m_pManager;
    ATTRIBUTES*     m_pARoot;

    IBIImage*     m_pIcon;
    storm::FPoint m_mousepos;
    storm::FPoint m_mousesensivity;
    storm::Rect   m_cursorzone;

    struct MouseCursorInfo {
        storm::Point offset;
        std::string  texture;
        storm::FRect uv;
    };

    MouseCursorInfo m_aCursors[BI_CURSORS_QUANTITY];
    int32_t         m_nCurrentCursor;
    storm::Point    m_cursorsize;

    void        InitMouseCursors();
    void        MoveCursor();
    void        SetCurrentCursor();
    storm::Rect GetCurrentCursorIconPos() const;
};
