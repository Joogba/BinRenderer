#pragma once

#include <vector>
#include <string>
#include <directxtk/SimpleMath.h>
#include "Vertex.h"

namespace BinRenderer {

    using DirectX::SimpleMath::Vector3;
    using DirectX::SimpleMath::Vector2;
    using std::vector;
    using std::string;

    // CPU-side mesh container: vertex arrays + indices + material metadata
    struct MeshData {
        vector<Vertex>         vertices;             // Static vertices
        vector<SkinnedVertex>  skinnedVertices;      // Skinned mesh vertices (if any)
        vector<uint32_t>       indices;              // Index list

        // Optional material metadata (file paths)
        string albedoTextureFilename;
        string emissiveTextureFilename;
        string normalTextureFilename;
        string heightTextureFilename;
        string aoTextureFilename;
        string metallicTextureFilename;
        string roughnessTextureFilename;
        string opacityTextureFilename;
    };

} // namespace BinCore