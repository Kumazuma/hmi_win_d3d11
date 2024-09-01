#include "graphics_system_d3d11.h"

#include <algorithm>
#include <graphics_element.h>
#include <stdexcept>
#include "graphics_element_pimpl.h"

#define STRINGIZE_DETAIL(x) #x
#define STRINGIZE(x) STRINGIZE_DETAIL(x)

namespace hmi_graphics
{
    SystemD3D11::SystemD3D11(HWND hWnd, int16_t width, int16_t height)
        : latestZIndexUpdated_{}
        , currentZIndexUpdated_{}
    {
        HRESULT hr;
        hr = CreateDXGIFactory1(__uuidof(IDXGIFactory2), &factory_);
        if(FAILED(hr))
            throw std::runtime_error(__FILE__ "::" STRINGIZE(__LINE__) " CreateDXGIFactory1");

        D3D_FEATURE_LEVEL featureLevel;
        hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0, D3D11_SDK_VERSION, &d3dDevice_, &featureLevel, &d3dContext_);
        if(FAILED(hr))
            throw std::runtime_error(__FILE__ "::" STRINGIZE(__LINE__) " D3D11CreateDevice");

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = width;
        swapChainDesc.Height = height;
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 2;
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        swapChainDesc.Flags = 0;

        hr = factory_->CreateSwapChainForHwnd(d3dDevice_.Get(), hWnd, &swapChainDesc, nullptr, nullptr, &swapChain_);
        if(FAILED(hr))
            throw std::runtime_error(__FILE__ "::" STRINGIZE(__LINE__) " CreateSwapChainForHwnd");

        ComPtr<IDXGIDevice> dxgiDevice;
        d3dDevice_.As(&dxgiDevice);

        D2D1CreateDevice(dxgiDevice.Get(), D2D1::CreationProperties(D2D1_THREADING_MODE_SINGLE_THREADED, D2D1_DEBUG_LEVEL_WARNING, D2D1_DEVICE_CONTEXT_OPTIONS_NONE), &d2dDevice_);
        d2dDevice_->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2dContextForElements_);
        d2dDevice_->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2dContextForRendering_);

        ComPtr<IDXGISurface> dxgiSurface;
        swapChain_->GetBuffer(0, __uuidof(dxgiSurface), &dxgiSurface);
        hr = d2dContextForRendering_->CreateBitmapFromDxgiSurface(dxgiSurface.Get(), D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)), &swapChainBitmap_);
        if(FAILED(hr))
            throw std::runtime_error(__FILE__ "::" STRINGIZE(__LINE__) " CreateBitmap");

        hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(decltype(dwriteFactory_)::InterfaceType), &dwriteFactory_);
        if(FAILED(hr))
            throw std::runtime_error(__FILE__ "::" STRINGIZE(__LINE__) " DWriteCreateFactory");
    }

    SystemD3D11::~SystemD3D11()
    {
        elements_.clear();
    }

    void SystemD3D11::RemoveElement(GraphicsElement* element)
    {
        auto it = elements_.begin();
        while (it != elements_.end())
        {
            auto& tuple = *it;
            if(std::get<0>(tuple) == element)
            {
                elements_.erase(it);
                break;
            }

            ++it;
        }
    }

    bool SystemD3D11::GetDirect2dDeviceContext(ID2D1DeviceContext** deviceContext)
    {
        if(deviceContext == nullptr)
        {
            return false;
        }

        *deviceContext = d2dContextForElements_.Get();
        d2dContextForElements_->AddRef();
        return true;
    }

    bool SystemD3D11::GetCachedColorBrush(const D2D1_COLOR_F& rgba, ID2D1SolidColorBrush** colorBrush)
    {
        uint32_t key = 0;
        key |= static_cast<int>(rgba.r * 255) % 256 << 24;
        key |= static_cast<int>(rgba.g * 255) % 256 << 16;
        key |= static_cast<int>(rgba.b * 255) % 256 << 8;
        key |= static_cast<int>(rgba.a * 255) % 256 << 0;
        for(auto& tuple : d2dColorBrushes_)
        {
            auto& color = std::get<0>(tuple);
            if(key == color)
            {
                auto& brush = std::get<1>(tuple);
                *colorBrush = brush.Get();
                brush->AddRef();
                return true;
            }
        }

        ComPtr<ID2D1SolidColorBrush> brush;
        d2dContextForElements_->CreateSolidColorBrush(rgba, &brush);
        *colorBrush = brush.Get();
        brush->AddRef();
        return true;
    }

    void SystemD3D11::Render()
    {
        for(auto& tuple: elements_)
        {
            auto* element = std::get<0>(tuple);
            if(!element->ResetUpdatedFlag())
                continue;

            element->Render(this);
        }

        if(currentZIndexUpdated_ != latestZIndexUpdated_)
        {
            std::stable_sort(elements_.begin(), elements_.end(), [](auto& e1, auto& e2)
            {
                GraphicsElement* lhs = std::get<0>(e1);
                GraphicsElement* rhs = std::get<0>(e2);
                return lhs->GetZIndex() < rhs->GetZIndex();
            });

            latestZIndexUpdated_ = currentZIndexUpdated_;
        }

        d2dContextForRendering_->SetTarget(swapChainBitmap_.Get());
        d2dContextForRendering_->BeginDraw();
        d2dContextForRendering_->Clear(D2D1::ColorF(D2D1::ColorF::White));
        for(auto& tuple: elements_)
        {
            auto& bitmap = std::get<2>(tuple);
            auto& element = std::get<0>(tuple);
            auto size = element->GetSize();
            auto pos = element->GetPosition();
            auto dest = D2D1::RectF(std::get<0>(pos), std::get<1>(pos));
            dest.right = dest.left + (float)std::get<0>(size);
            dest.bottom = dest.top + (float)std::get<1>(size);
            d2dContextForRendering_->DrawBitmap(bitmap.Get(), dest);
        }

        d2dContextForRendering_->EndDraw();
        swapChain_->Present(1, 0);
    }

    void SystemD3D11::GetDirect3dDevice(ID3D11Device** device)
    {
        *device = d3dDevice_.Get();
        d3dDevice_->AddRef();
    }

    void SystemD3D11::GetDirect3dContext(ID3D11DeviceContext** deviceContext)
    {
        *deviceContext = d3dContext_.Get();
        d3dContext_->AddRef();
    }

    void SystemD3D11::GetDirectWriteFactory(IDWriteFactory** factory)
    {
        *factory = dwriteFactory_.Get();
        dwriteFactory_->AddRef();
    }

    GraphicsElement* SystemD3D11::HitTest(int32_t x, int32_t y, GraphicsElement* hint)
    {
        auto iter = elements_.begin();
        if(hint != nullptr)
        {
            while(iter != elements_.end())
            {
                auto& tuple = *iter;
                auto* element = std::get<0>(tuple);
                if(element == hint)
                {
                    ++iter;
                    break;
                }

                ++iter;
            }
        }

        GraphicsElement* result = nullptr;
        for(;iter != elements_.end(); ++iter)
        {

            auto& tuple = *iter;
            auto* element = std::get<0>(tuple);
            int16_t posX;
            int16_t posY;
            int16_t width;
            int16_t height;
            std::tie(posX, posY) = element->GetPosition();
            if(posX > x || posY > y)
                continue;

            std::tie(width, height) = element->GetSize();
            posX += width;
            posY += height;
            if(posX < x || posY < y)
                continue;

            result = element;
            break;
        }

        return result;
    }

    void SystemD3D11::ElementZIndexUpdated()
    {
        currentZIndexUpdated_ += 1;
    }

    void SystemD3D11::AddElement(GraphicsElement* element, int16_t width, int16_t height)
    {
        ComPtr<ID3D11Texture2D> texture;
        D3D11_TEXTURE2D_DESC desc{};
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_RENDER_TARGET;
        d3dDevice_->CreateTexture2D(&desc, nullptr, &texture);
        element->Initialize(new GraphicsElement::Pimpl{this, width, height, texture.Get()}, this);
        ComPtr<IDXGISurface> surface;
        ComPtr<ID2D1Bitmap1> bitmapSource;
        texture->QueryInterface(IID_PPV_ARGS(&surface));
        auto sourceProp = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_NONE, D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
        d2dContextForRendering_->CreateBitmapFromDxgiSurface(surface.Get(), sourceProp, &bitmapSource);

        // auto destProp = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
        // d2dContextForElements_->CreateBitmapFromDxgiSurface(surface.Get(), destProp, &bitmapTarget);
        elements_.emplace_back(element, texture, bitmapSource);
        ElementZIndexUpdated();
    }
}
