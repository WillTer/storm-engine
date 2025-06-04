#include "editor_defines.h"

#include <cstdint>

void GIEditorObject::LinkEvent(GIEditorEventHandler* pEventHandler, GIEditorEvent const& pEventFunction)
{
    pEventHandler->AddEventFunction(this, pEventFunction);
}

GIEditorEventHandler::GIEditorEventHandler() {}

GIEditorEventHandler::~GIEditorEventHandler()
{
    m_aEventFuncs.clear();
}

bool GIEditorEventHandler::Execute()
{
    for (int32_t n = 0; n < m_aEventFuncs.size(); n++) {
        (m_aEventFuncs[n].pObj->*m_aEventFuncs[n].func)();
    }
    return true;
}

void GIEditorEventHandler::AddEventFunction(GIEditorObject* pObj, GIEditorEvent const& pEventFunction)
{
    FuncDescr fd;
    fd.pObj = pObj;
    fd.func = pEventFunction;
    m_aEventFuncs.push_back(fd);
}
