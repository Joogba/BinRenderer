#pragma once

#include <vector>
#include <string>
#include "Core/Vertex.h"

namespace BinRenderer {

    // CPU-side mesh container: vertex arrays + indices + material metadata
    struct MeshData {
        std::vector<Vertex>    vertices;     // Static vertices
     std::vector<SkinnedVertex>  skinnedVertices;      // Skinned mesh vertices (if any)
        std::vector<uint32_t>       indices;    // Index list

        // Optional material metadata (file paths)
        std::string albedoTextureFilename;
      std::string emissiveTextureFilename;
        std::string normalTextureFilename;
        std::string heightTextureFilename;
        std::string aoTextureFilename;
  std::string metallicTextureFilename;
        std::string roughnessTextureFilename;
        std::string opacityTextureFilename;
    };

} // namespace BinRenderer