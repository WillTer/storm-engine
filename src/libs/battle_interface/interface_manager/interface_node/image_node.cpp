#include "image_node.h"

#include <libs/core/core.h>

#include "../../image/img_render.h"

BI_ImageNode::BI_ImageNode(
    BI_ManagerBase* pManager, char const* texture, storm::FRect const& uv, storm::Rect const& pos, uint32_t color, int32_t nPrioritet)
    : BI_BaseNode(pManager)
{
    m_pImage = Manager()->GetImageRender()->CreateImage(BIType_square, texture, color, uv, pos, nPrioritet);
}

BI_ImageNode::~BI_ImageNode()
{
    STORM_DELETE(m_pImage);
}

void BI_ImageNode::Update()
{
    BI_BaseNode::Update();
}
