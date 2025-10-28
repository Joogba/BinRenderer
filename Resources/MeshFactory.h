#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>

#include "MeshData.h"
#include "Core/Vertex.h"

namespace BinRenderer {

    class MeshFactory {
    public:
        // Creates MeshData (CPU-side vertex/index data) 
        static MeshData CreateQuadMesh(float scale = 1.0f);
        
        static MeshData CreateCube(float size = 1.0f);
        
        static MeshData CreatePlane(float size = 10.0f);
        
        static MeshData CreateSphere(float radius = 1.0f, uint32_t slices = 32, uint32_t stacks = 16);
    };

} // namespace BinRenderer
