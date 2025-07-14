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
    };

    struct InstancingKey {
        PSOHandle pso;
        MaterialHandle material;
        MeshHandle mesh;
        bool operator==(const InstancingKey& rhs) const {
            return pso == rhs.pso && material == rhs.material && mesh == rhs.mesh;
        }
    };
}

namespace std {

    template<>
    struct hash<BinRenderer::InstancingKey> {
        size_t operator()(const BinRenderer::InstancingKey& k) const {
            return std::hash<uint16_t>()(k.pso.idx) ^
                (std::hash<uint16_t>()(k.material.idx) << 1) ^
                (std::hash<uint16_t>()(k.mesh.idx) << 2);
        }
    };
}