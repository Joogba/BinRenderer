#pragma once
#include <cstdint>
#include <vector>
#include <glm/glm.hpp>

#include "Handle.h"

namespace BinRenderer
{
    struct DrawCommand
    {
        uint8_t                 viewId = 0;
        MeshHandle              meshHandle;
        MaterialHandle          materialHandle;
        PSOHandle               psoHandle;
        glm::mat4               transform;
        uint64_t                sortKey = 0;

        uint32_t                instanceCount = 1;
        uint32_t                instanceOffset = 0;
        std::vector<glm::mat4>  transforms;
    };
}