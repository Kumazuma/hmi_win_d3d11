#ifndef GRAPHICS_ELEMENT_PIMPL_H
#define GRAPHICS_ELEMENT_PIMPL_H

#include "comptr.h"
#include "graphics_element.h"
#include "graphics_system.h"
#include "graphics_system_d3d11.h"
#include <d3d11.h>
#include <d2d1_2.h>
#include <wrl.h>


class hmi_graphics::GraphicsElement::Pimpl
{

public:
    Pimpl(System* system, int16_t width, int16_t height, ID3D11Texture2D* texture);

    bool GetTarget(ID2D1Bitmap1** target);

private:
    ComPtr<ID2D1Bitmap1> target_;
    ComPtr<ID3D11Texture2D> targetTexture_;
    ComPtr<ID2D1DeviceContext> context_;
};

inline hmi_graphics::GraphicsElement::Pimpl::Pimpl(System* system, int16_t width, int16_t height, ID3D11Texture2D* texture)
{
    auto d3d11System = static_cast<SystemD3D11*>(system);


    ComPtr<IDXGISurface> surface;
    targetTexture_->QueryInterface(IID_PPV_ARGS(&surface));

}

inline bool hmi_graphics::GraphicsElement::Pimpl::GetTarget(ID2D1Bitmap1** target)
{
    if(target_)
    {
        *target = target_.Get();
        target_->AddRef();
        return true;
    }

    return false;
}

#endif //GRAPHICS_ELEMENT_PIMPL_H
