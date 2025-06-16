#pragma once

#include <d3d11.h>
#include <memory>
#include "MeshRegistry.h"
#include "MeshData.h"

namespace BinRenderer {

    class MeshFactory {
    public:
        // Creates a Mesh (vertex/index GPU buffers) from CPU-side MeshData
        static std::unique_ptr<Mesh> CreateMeshFromData(
            ID3D11Device* device,
            const MeshData& meshData);

        // Shortcut for a unit quad
        static std::unique_ptr<Mesh> CreateQuadMesh(
            ID3D11Device* device,
            float scale = 1.0f);
    };

} // namespace BinRenderer
