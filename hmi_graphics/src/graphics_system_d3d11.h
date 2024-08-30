#ifndef GRAPHICS_SYSTEM_D3D11_H
#define GRAPHICS_SYSTEM_D3D11_H

#include <vector>
#include <tuple>
#include <dxgi1_5.h>
#include "comptr.h"
#include "graphics_system.h"

namespace hmi_graphics
{
    class SystemD3D11: public System
    {
    public:
        SystemD3D11(HWND hWnd, int16_t width, int16_t height);

        ~SystemD3D11() override;

        void RemoveElement(GraphicsElement* element) override;

        bool GetDirect2dDeviceContext(ID2D1DeviceContext** deviceContext) override;

        bool GetCachedColorBrush(const D2D1_COLOR_F& rgba, ID2D1SolidColorBrush** colorBrush) override;

        void Render() override;

        void GetDirect3dDevice(ID3D11Device** device);

        void GetDirect3dDeviceContext(ID3D11DeviceContext** deviceContext);

    protected:
        void AddElement(GraphicsElement* element, int16_t width, int16_t height) override;

    private:
        std::vector<std::tuple<GraphicsElement*, ComPtr<ID3D11Texture2D>, ComPtr<ID2D1Bitmap1>>> m_elements;
        ComPtr<ID3D11Device> d3dDevice_;
        ComPtr<ID3D11DeviceContext> d3dContext_;
        ComPtr<IDXGISwapChain1> swapChain_;
        ComPtr<IDXGIFactory2> factory_;
        ComPtr<ID2D1Bitmap1> swapChainBitmap_;
        ComPtr<ID2D1Device> d2dDevice_;
        ComPtr<ID2D1Factory> d2dFactory_;
        ComPtr<ID2D1DeviceContext> d2dContextForElements_;
        ComPtr<ID2D1DeviceContext> d2dContextForRendering_;
    };
}

#endif //GRAPHICS_SYSTEM_D3D11_H
