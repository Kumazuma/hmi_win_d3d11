#include "graphics_element.h"
#include "graphics_element_pimpl.h"

namespace hmi_graphics
{
    GraphicsElement::GraphicsElement()
        : pimpl_{}
    {
    }

    bool GraphicsElement::Initialize(Pimpl* pimpl, System* parent)
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

    int16_t GraphicsElement::GetZIndex() const
    {
        return pimpl_->zIndex_;
    }

    void GraphicsElement::SetZIndex(int16_t zIndex)
    {
        int16_t oldZIndex = pimpl_->zIndex_;
        pimpl_->zIndex_ = zIndex;
        if(oldZIndex != zIndex)
        {
            pimpl_->system_->ElementZIndexUpdated();
        }
    }

    void GraphicsElement::SetSize(int16_t width, int16_t height)
    {
        pimpl_->width_ = width;
        pimpl_->height_ = height;
    }

    std::tuple<int16_t, int16_t> GraphicsElement::GetSize() const
    {
        return {pimpl_->width_, pimpl_->height_};
    }

    void GraphicsElement::SetPosition(int16_t x, int16_t y)
    {
        pimpl_->x_ = x;
        pimpl_->y_ = y;
    }

    std::tuple<int16_t, int16_t> GraphicsElement::GetPosition() const
    {
        return {pimpl_->x_, pimpl_->y_};
    }

    bool GraphicsElement::GetTarget(ID2D1Bitmap1** target)
    {
        if(target == nullptr)
        {
            return false;
        }

        return pimpl_->GetTarget(target);
    }

    System* GraphicsElement::GetParent() const
    {
        return pimpl_->system_;
    }

    void GraphicsElement::NotifyUpdated()
    {
        pimpl_->updated_ = true;
    }

    bool GraphicsElement::ResetUpdatedFlag()
    {
        bool updated = pimpl_->updated_;
        pimpl_->updated_ = false;
        return updated;
    }

    ID2D1Bitmap1* GraphicsElement::GetTarget() const
    {
        return pimpl_->GetTarget();
    }
}
