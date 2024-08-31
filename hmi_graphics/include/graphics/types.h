#ifndef HMI_GRAPHICS_TYPES_H
#define HMI_GRAPHICS_TYPES_H

namespace hmi_graphics
{
    struct Point
    {
      int x;
      int y;
    };

    struct Size
    {
      int width;
      int height;
    };

    struct Rect
    {
      Point origin;
      Size size;
    };
}

#endif //HMI_GRAPHICS_TYPES_H
