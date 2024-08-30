#include "graphics_system_d3d11.h"

#include <graphics_element.h>

#include "graphics_element_pimpl.h"

namespace hmi_graphics
{
    SystemD3D11::SystemD3D11(HWND hWnd, int16_t width, int16_t height)
    {

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
        return false;
    }

    bool SystemD3D11::GetCachedColorBrush(const D2D1_COLOR_F& rgba, ID2D1SolidColorBrush** colorBrush)
    {
        return false;
    }

    void SystemD3D11::Render()
    {

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
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
        d3dDevice_->CreateTexture2D(&desc, nullptr, &texture);
        element->Initialize(new GraphicsElement::Pimpl{this, width, height, texture.Get()});
    }
}
