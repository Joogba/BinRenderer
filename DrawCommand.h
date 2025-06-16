#pragma once
#include <DirectXMath.h>
#include <cstdint>

#include "Handle.h"

namespace BinRenderer
{
    struct DrawCommand
    {
        MeshHandle meshHandle;
        MaterialHandle materialHandle;
        PSOHandle pso;
        DirectX::XMMATRIX transform;
    };
}