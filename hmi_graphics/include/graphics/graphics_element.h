#ifndef GRAPHICS_ELEMENT_H
#define GRAPHICS_ELEMENT_H

#include <cstdint>
#include <tuple>
#include <d2d1_2.h>

#if defined(_WIN32) && defined(HMI_GRAPHICS_DLL)
#if !defined(HMI_GRAPHICS_EXPORT)
#define HMI_GRAPHICS_EXPORT __declspec(dllexport)
#endif
#else
#define HMI_GRAPHICS_EXPORT
#endif

namespace hmi_graphics
{
    class System;
    class HMI_GRAPHICS_EXPORT GraphicsElement
    {
    public:
        class Pimpl;

        explicit GraphicsElement();

        GraphicsElement(const GraphicsElement&) = delete;

        virtual bool Initialize(Pimpl* pimpl);

        virtual ~GraphicsElement();

        int16_t GetZIndex() const;

        void SetZIndex(int16_t zIndex);

        void SetSize(int16_t width, int16_t height);

        std::tuple<int16_t, int16_t> GetSize() const;

        void SetPosition(int16_t x, int16_t y);

        std::tuple<int16_t, int16_t> GetPosition() const;

        bool GetTarget(ID2D1Bitmap1** target);

        virtual void Render(System* parent) = 0;
    private:
        Pimpl *pimpl_;
    };

}

#endif //GRAPHICS_ELEMENT_H
