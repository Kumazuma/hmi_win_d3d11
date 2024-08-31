#ifndef GRAPHICS_ELEMENT_PIMPL_H
#define GRAPHICS_ELEMENT_PIMPL_H

#include "comptr.h"
#include "graphics_element.h"
#include "graphics_system.h"
#include "graphics_system_d3d11.h"
#include <d3d11.h>
#include <d2d1_2.h>
#include <wrl.h>
#include <cassert>

class hmi_graphics::GraphicsElement::Pimpl
{
    friend class hmi_graphics::GraphicsElement;
public:
    Pimpl(System* system, int16_t width, int16_t height, ID3D11Texture2D* texture);

    bool GetTarget(ID2D1Bitmap1** target);

    ID2D1Bitmap1* GetTarget();

private:
    int16_t x_;
    int16_t y_;
    int16_t width_;
    int16_t height_;
    int16_t zIndex_;
    SystemD3D11* system_;
    ComPtr<ID2D1Bitmap1> target_;
    ComPtr<ID3D11Texture2D> targetTexture_;
    ComPtr<ID2D1DeviceContext> context_;
};

inline hmi_graphics::GraphicsElement::Pimpl::Pimpl(System* system, int16_t width, int16_t height, ID3D11Texture2D* texture)
{
    HRESULT hr{};
    system_ = static_cast<SystemD3D11*>(system);
    ComPtr<IDXGISurface> surface;
    ComPtr<ID2D1DeviceContext> context;
    system_->GetDirect2dDeviceContext(&context);
    targetTexture_ = texture;
    width_ = width;
    height_ = height;
    x_ = 0;
    y_ = 0;
    targetTexture_->QueryInterface(IID_PPV_ARGS(&surface));
    auto destProp = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
    hr = context->CreateBitmapFromDxgiSurface(surface.Get(), destProp, &target_);
    assert(SUCCEEDED(hr));
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

inline ID2D1Bitmap1* hmi_graphics::GraphicsElement::Pimpl::GetTarget()
{
    return target_.Get();
}

#endif //GRAPHICS_ELEMENT_PIMPL_H
