#include "graphics_system.h"
#include "graphics_system_d3d11.h"
#include <new>

namespace hmi_graphics
{
    System* System::CreateInstance(HWND hWnd, int16_t width, int16_t height)
    {
        return new(std::nothrow) SystemD3D11{hWnd, width, height};
    }
}
