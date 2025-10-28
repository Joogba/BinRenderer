#include "MeshFactory.h"
#include <numbers>

namespace BinRenderer {

    MeshData MeshFactory::CreateQuadMesh(float scale) {
        MeshData meshData;
        
        float h = scale * 0.5f;
        
        // Vertices
        meshData.vertices = {
            // Position            Normal      UV
            {{-h, -h, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // Bottom-left
            {{+h, -h, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // Bottom-right
            {{+h, +h, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // Top-right
            {{-h, +h, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}  // Top-left
        };
        
        // Indices
        meshData.indices = {
            0, 1, 2,
            0, 2, 3
        };
        
        return meshData;
    }

    MeshData MeshFactory::CreateCube(float size) {
        MeshData meshData;
        
        float h = size * 0.5f;
        
        // 24 vertices (4 per face, 6 faces)
        meshData.vertices = {
            // +X face (Right)
            {{+h, -h, -h}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            {{+h, +h, -h}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{+h, +h, +h}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{+h, -h, +h}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            
            // -X face (Left)
            {{-h, -h, +h}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            {{-h, +h, +h}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{-h, +h, -h}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-h, -h, -h}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            
            // +Y face (Top)
            {{-h, +h, -h}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
            {{-h, +h, +h}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{+h, +h, +h}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{+h, +h, -h}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
            
            // -Y face (Bottom)
            {{-h, -h, +h}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
            {{-h, -h, -h}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
            {{+h, -h, -h}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
            {{+h, -h, +h}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
            
            // +Z face (Front)
            {{-h, -h, +h}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{+h, -h, +h}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{+h, +h, +h}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{-h, +h, +h}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            
            // -Z face (Back)
            {{+h, -h, -h}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
            {{-h, -h, -h}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
            {{-h, +h, -h}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
            {{+h, +h, -h}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}}
        };
        
        // 36 indices (6 faces * 2 triangles * 3 vertices)
        meshData.indices = {
            // +X
            0, 1, 2,    0, 2, 3,
            // -X
            4, 5, 6,    4, 6, 7,
            // +Y
            8, 9, 10,   8, 10, 11,
            // -Y
            12, 13, 14, 12, 14, 15,
            // +Z
            16, 17, 18, 16, 18, 19,
            // -Z
            20, 21, 22, 20, 22, 23
        };
        
        return meshData;
    }

    MeshData MeshFactory::CreatePlane(float size) {
        MeshData meshData;
        
        float h = size * 0.5f;
        
        // Vertices (XZ plane, Y-up)
        meshData.vertices = {
            {{-h, 0.0f, -h}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
            {{+h, 0.0f, -h}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
            {{+h, 0.0f, +h}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{-h, 0.0f, +h}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}
        };
        
        // Indices
        meshData.indices = {
            0, 1, 2,
            0, 2, 3
        };
        
        return meshData;
    }

    MeshData MeshFactory::CreateSphere(float radius, uint32_t slices, uint32_t stacks) {
        MeshData meshData;
        
        // Generate vertices
        for (uint32_t stack = 0; stack <= stacks; ++stack) {
            float phi = std::numbers::pi_v<float> * static_cast<float>(stack) / static_cast<float>(stacks);
            float y = radius * std::cos(phi);
            float r = radius * std::sin(phi);
            
            for (uint32_t slice = 0; slice <= slices; ++slice) {
                float theta = 2.0f * std::numbers::pi_v<float> * static_cast<float>(slice) / static_cast<float>(slices);
                float x = r * std::cos(theta);
                float z = r * std::sin(theta);
                
                glm::vec3 position(x, y, z);
                glm::vec3 normal = glm::normalize(position);
                glm::vec2 uv(
                    static_cast<float>(slice) / static_cast<float>(slices),
                    static_cast<float>(stack) / static_cast<float>(stacks)
                );
                
                meshData.vertices.push_back({position, normal, uv});
            }
        }
        
        // Generate indices
        for (uint32_t stack = 0; stack < stacks; ++stack) {
            for (uint32_t slice = 0; slice < slices; ++slice) {
                uint32_t first = stack * (slices + 1) + slice;
                uint32_t second = first + slices + 1;
                
                // First triangle
                meshData.indices.push_back(first);
                meshData.indices.push_back(second);
                meshData.indices.push_back(first + 1);
                
                // Second triangle
                meshData.indices.push_back(second);
                meshData.indices.push_back(second + 1);
                meshData.indices.push_back(first + 1);
            }
        }
        
        return meshData;
    }

} // namespace BinRenderer