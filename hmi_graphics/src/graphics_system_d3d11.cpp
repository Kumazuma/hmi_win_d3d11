#include "graphics_system_d3d11.h"

#include <graphics_element.h>

#include "graphics_element_pimpl.h"

#define STRINGIZE_DETAIL(x) #x
#define STRINGIZE(x) STRINGIZE_DETAIL(x)

namespace hmi_graphics
{
    SystemD3D11::SystemD3D11(HWND hWnd, int16_t width, int16_t height)
    {
        HRESULT hr;
        hr = CreateDXGIFactory1(__uuidof(IDXGIFactory2), &factory_);
        if(FAILED(hr))
            throw std::exception(__FILE__ "::" STRINGIZE(__LINE__) " CreateDXGIFactory1");

        D3D_FEATURE_LEVEL featureLevel;
        hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0, D3D11_SDK_VERSION, &d3dDevice_, &featureLevel, &d3dContext_);
        if(FAILED(hr))
            throw std::exception(__FILE__ "::" STRINGIZE(__LINE__) " D3D11CreateDevice");

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
            throw std::exception(__FILE__ "::" STRINGIZE(__LINE__) " CreateSwapChainForHwnd");

        ComPtr<IDXGIDevice> dxgiDevice;
        d3dDevice_.As(&dxgiDevice);
        D2D1CreateDevice(dxgiDevice.Get(), D2D1::CreationProperties(D2D1_THREADING_MODE_SINGLE_THREADED, D2D1_DEBUG_LEVEL_WARNING, D2D1_DEVICE_CONTEXT_OPTIONS_NONE), &d2dDevice_);
        d2dDevice_->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2dContextForElements_);
        d2dDevice_->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2dContextForRendering_);

        ComPtr<IDXGISurface> dxgiSurface;
        swapChain_->GetBuffer(0, __uuidof(dxgiSurface), &dxgiSurface);
        hr = d2dContextForRendering_->CreateBitmapFromDxgiSurface(dxgiSurface.Get(), D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)), &swapChainBitmap_);
        if(FAILED(hr))
            throw std::exception(__FILE__ "::" STRINGIZE(__LINE__) " CreateBitmap");
    }

    SystemD3D11::~SystemD3D11()
    {
        m_elements.clear();
    }

    void SystemD3D11::RemoveElement(GraphicsElement* element)
    {
        auto it = m_elements.begin();
        while (it != m_elements.end())
        {
            auto& tuple = *it;
            if(std::get<0>(tuple) == element)
            {
                m_elements.erase(it);
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
        for(auto& tuple : d2dColorBrushes_)
        {
            auto& color = std::get<0>(tuple);
            if(color.a == rgba.a && color.b == rgba.b && color.g == rgba.g && color.r == rgba.r)
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
        for(auto& tuple: m_elements)
        {
            auto* element = std::get<0>(tuple);
            element->Render(this);
        }

        d2dContextForRendering_->SetTarget(swapChainBitmap_.Get());
        d2dContextForRendering_->BeginDraw();
        d2dContextForRendering_->Clear(D2D1::ColorF(D2D1::ColorF::White));
        for(auto& tuple: m_elements)
        {
            auto& bitmap = std::get<2>(tuple);
            d2dContextForRendering_->DrawBitmap(bitmap.Get());
        }

        d2dContextForRendering_->EndDraw();
        swapChain_->Present(1, 0);
    }

    void SystemD3D11::GetDirect3dDevice(ID3D11Device** device)
    {

    }

    void SystemD3D11::GetDirect3dDeviceContext(ID3D11DeviceContext** deviceContext)
    {

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
        m_elements.emplace_back(element, texture, bitmapSource);
    }
}
