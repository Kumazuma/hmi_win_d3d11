#ifndef GURUM_GRAPHICS_SYSTEM_LIBRARY_H
#define GURUM_GRAPHICS_SYSTEM_LIBRARY_H

#include <cstdint>
#include <tuple>
#include <Windows.h>
#include <d2d1_2.h>
#include <d3d11.h>
#include <dwrite.h>

#if defined(_WIN32) && defined(HMI_GRAPHICS_DLL)
#if !defined(HMI_GRAPHICS_EXPORT)
#define HMI_GRAPHICS_EXPORT __declspec(dllexport)
#endif
#else
#define HMI_GRAPHICS_EXPORT
#endif

namespace hmi_graphics
{
    class GraphicsElement;
    class System
    {
    public:
        static HMI_GRAPHICS_EXPORT System* CreateInstance(HWND hWnd, int16_t width, int16_t height);

        virtual ~System() = default;

        template<typename T, typename... Args>
        T* AddElement(int16_t width, int16_t height, Args&&... args);

        virtual void RemoveElement(GraphicsElement* element) = 0;

        virtual bool GetDirect2dDeviceContext(ID2D1DeviceContext** deviceContext) = 0;

        virtual bool GetCachedColorBrush(const D2D1_COLOR_F& rgba, ID2D1SolidColorBrush** colorBrush) = 0;

        virtual void Render() = 0;

        virtual void GetDirect3dDevice(ID3D11Device** device) = 0;

        virtual void GetDirect3dContext(ID3D11DeviceContext** context) = 0;

        virtual void GetDirectWriteFactory(IDWriteFactory** factory) = 0;

    protected:
        virtual void AddElement(GraphicsElement* element, int16_t width, int16_t height) = 0;

    };

    template <typename T, typename... Args>
    inline T* System::AddElement(int16_t width, int16_t height, Args&&... args)
    {
        auto element = new T{ std::forward<Args>(args)... };
        AddElement(element, width, height);
        return element;
    }
}

#endif //GURUM_GRAPHICS_SYSTEM_LIBRARY_H
