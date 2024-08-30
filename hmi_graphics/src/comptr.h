#ifndef HMI_COMPTR_H
#define HMI_COMPTR_H

#include <wrl.h>
namespace hmi_graphics
{
    template<typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;
}

#endif //HMI_COMPTR_H
