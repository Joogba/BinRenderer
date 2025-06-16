#pragma once
#include <DirectXMath.h>
#include <cstdint>

#include "Handle.h"

namespace BinRenderer
{
    struct DrawCommand
    {
        uint8_t             viewId = 0;
        MeshHandle          meshHandle;
        MaterialHandle      materialHandle;
        PSOHandle           psoHandle;
        DirectX::XMMATRIX   transform;
    };
}