#include "graphics_element.h"
#include "graphics_element_pimpl.h"

namespace hmi_graphics
{
    GraphicsElement::GraphicsElement()
        : pimpl_{}
    {
    }

    bool GraphicsElement::Initialize(Pimpl* pimpl)
    {
        if (pimpl == nullptr)
        {
            return false;
        }

        pimpl_ = pimpl;
        return true;
    }

    GraphicsElement::~GraphicsElement()
    {
        delete pimpl_;
        pimpl_ = nullptr;
    }

    bool GraphicsElement::GetTarget(ID2D1Bitmap1** target)
    {
        if(target == nullptr)
        {
            return false;
        }

        return pimpl_->GetTarget(target);
    }
}
