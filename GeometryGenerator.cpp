#include "GeometryGenerator.h"
#include <cmath>

namespace BinRenderer {

    MeshData GeometryGenerator::MakeSquare(float scale,
        const Vector2& texScale) {
        MeshData meshData;
        meshData.vertices.reserve(4);

        Vector3 positions[4] = {
            { -1.0f,  1.0f, 0.0f },
            {  1.0f,  1.0f, 0.0f },
            {  1.0f, -1.0f, 0.0f },
            { -1.0f, -1.0f, 0.0f }
        };
        Vector3 normal = { 0.0f, 0.0f, -1.0f };
        Vector2 uvs[4] = {
            { 0.0f, 0.0f },
            { 1.0f, 0.0f },
            { 1.0f, 1.0f },
            { 0.0f, 1.0f }
        };

        for (int i = 0; i < 4; ++i) {
            Vertex v;
            v.position = positions[i] * scale;
            v.normalModel = normal;
            v.texcoord = uvs[i] * texScale;
            v.tangentModel = { 1.0f, 0.0f, 0.0f };
            meshData.vertices.push_back(v);
        }
        meshData.indices = { 0,1,2, 0,2,3 };
        return meshData;
    }

    MeshData GeometryGenerator::MakeSquareGrid(int numSlices,
        int numStacks,
        float scale,
        const Vector2& texScale) {
        MeshData meshData;
        float dx = 2.0f / numSlices;
        float dy = 2.0f / numStacks;
        meshData.vertices.reserve((numSlices + 1) * (numStacks + 1));

        for (int j = 0; j <= numStacks; ++j) {
            float y = 1.0f - dy * j;
            for (int i = 0; i <= numSlices; ++i) {
                float x = -1.0f + dx * i;
                Vertex v;
                v.position = Vector3(x, y, 0.0f) * scale;
                v.normalModel = { 0.0f, 0.0f, -1.0f };
                v.texcoord = Vector2((x + 1.0f) * 0.5f, (y + 1.0f) * 0.5f) * texScale;
                v.tangentModel = { 1.0f, 0.0f, 0.0f };
                meshData.vertices.push_back(v);
            }
        }
        for (int j = 0; j < numStacks; ++j) {
            int row0 = (numSlices + 1) * j;
            for (int i = 0; i < numSlices; ++i) {
                meshData.indices.push_back(row0 + i);
                meshData.indices.push_back(row0 + i + 1);
                meshData.indices.push_back(row0 + i + numSlices + 1);

                meshData.indices.push_back(row0 + i + numSlices + 1);
                meshData.indices.push_back(row0 + i + 1);
                meshData.indices.push_back(row0 + i + numSlices + 2);
            }
        }
        return meshData;
    }

    // Other generators can be implemented similarly...

} // namespace BinRenderer
