// GeometryGenerator.cpp (updated)
#include "GeometryGenerator.h"
#include <cmath>

namespace BinCore {

    using namespace DirectX;
    using DirectX::SimpleMath::Matrix;
    using DirectX::SimpleMath::Vector3;
    using DirectX::SimpleMath::Vector2;

    MeshData GeometryGenerator::MakeSquare(const float scale,
        const Vector2 texScale) {
        std::vector<Vector3> positions;
        std::vector<Vector3> normals;
        std::vector<Vector2> texcoords;

        // ¾Õ¸é
        positions = {
            Vector3(-1.0f,  1.0f, 0.0f) * scale,
            Vector3(1.0f,  1.0f, 0.0f) * scale,
            Vector3(1.0f, -1.0f, 0.0f) * scale,
            Vector3(-1.0f, -1.0f, 0.0f) * scale
        };
        normals.assign(4, Vector3(0.0f, 0.0f, -1.0f));
        texcoords = {
            Vector2(0.0f, 0.0f),
            Vector2(1.0f, 0.0f),
            Vector2(1.0f, 1.0f),
            Vector2(0.0f, 1.0f)
        };

        MeshData meshData;
        meshData.vertices.reserve(4);
        for (size_t i = 0; i < positions.size(); ++i) {
            Vertex v;
            v.position = positions[i];
            v.normalModel = normals[i];
            v.texcoord = texcoords[i] * texScale;
            v.tangentModel = Vector3(1.0f, 0.0f, 0.0f);
            meshData.vertices.push_back(v);
        }
        meshData.indices = { 0,1,2, 0,2,3 };
        return meshData;
    }

    MeshData GeometryGenerator::MakeSquareGrid(const int numSlices,
        const int numStacks,
        const float scale,
        const Vector2 texScale) {
        MeshData meshData;
        float dx = 2.0f / numSlices;
        float dy = 2.0f / numStacks;
        float y = 1.0f;

        for (int j = 0; j <= numStacks; ++j) {
            float x = -1.0f;
            for (int i = 0; i <= numSlices; ++i) {
                Vertex v;
                v.position = Vector3(x, y, 0.0f) * scale;
                v.normalModel = Vector3(0.0f, 0.0f, -1.0f);
                v.texcoord = Vector2((x + 1.0f) * 0.5f, (y + 1.0f) * 0.5f) * texScale;
                v.tangentModel = Vector3(1.0f, 0.0f, 0.0f);
                meshData.vertices.push_back(v);
                x += dx;
            }
            y -= dy;
        }
        for (int j = 0; j < numStacks; ++j) {
            for (int i = 0; i < numSlices; ++i) {
                int row0 = (numSlices + 1) * j;
                meshData.indices.push_back(row0 + i);
                meshData.indices.push_back(row0 + i + 1);
                meshData.indices.push_back(row0 + numSlices + 1 + i);
                meshData.indices.push_back(row0 + numSlices + 1 + i);
                meshData.indices.push_back(row0 + i + 1);
                meshData.indices.push_back(row0 + numSlices + 1 + i + 1);
            }
        }
        return meshData;
    }

    MeshData GeometryGenerator::MakeGrass() {
        MeshData grid = MakeSquareGrid(1, 4);
        for (auto& v : grid.vertices) {
            v.position.x *= 0.02f;
            v.position.y = v.position.y * 0.5f + 0.5f;
        }
        // simplify top into a point
        grid.indices.erase(grid.indices.begin(), grid.indices.begin() + 3);
        for (auto& idx : grid.indices) idx -= 1;
        grid.vertices.erase(grid.vertices.begin());
        grid.vertices[0].position.x = 0.0f;
        grid.vertices[0].texcoord.x = 0.5f;
        return grid;
    }

    MeshData GeometryGenerator::MakeBox(const float scale) {
        // similar to MakeSquare but with 6 faces
        // omitted for brevity
        return MakeIcosahedron(); // placeholder or actual implementation
    }

    MeshData GeometryGenerator::MakeWireBox(const Vector3 center,
        const Vector3 extents) {
        MeshData meshData;
        std::vector<Vector3> positions;
        // front/back/side loops
        // omitted for brevity
        return meshData;
    }

    MeshData GeometryGenerator::MakeWireSphere(const Vector3 center,
        const float radius) {
        MeshData meshData;
        const int numPoints = 30;
        float dTheta = XM_2PI / numPoints;
        // generate circles in XY, YZ, XZ
        // omitted for brevity
        return meshData;
    }

    MeshData GeometryGenerator::MakeCylinder(const float bottomRadius,
        const float topRadius,
        float height,
        int numSlices) {
        MeshData meshData;
        float dTheta = -XM_2PI / numSlices;
        // generate side vertices and indices
        // omitted
        return meshData;
    }

    MeshData GeometryGenerator::MakeSphere(const float radius,
        const int numSlices,
        const int numStacks,
        const Vector2 texScale) {
        MeshData meshData;
        float dTheta = -XM_2PI / numSlices;
        float dPhi = -XM_PI / numStacks;
        // generate stacks and slices
        // omitted
        return meshData;
    }

    MeshData GeometryGenerator::MakeTetrahedron() {
        MeshData meshData;
        // generate points and indices
        return meshData;
    }

    MeshData GeometryGenerator::MakeIcosahedron() {
        MeshData meshData;
        // generate 12 vertices and 60 indices
        return meshData;
    }

    MeshData GeometryGenerator::SubdivideToSphere(const float radius,
        MeshData meshData) {
        for (auto& v : meshData.vertices) {
            v.position = v.normalModel * radius;
            v.normalModel.Normalize();
        }
        MeshData newMesh;
        // subdivide logic
        return newMesh;
    }

    void GeometryGenerator::Normalize(const Vector3 center,
        const float longestLength,
        std::vector<MeshData>& meshes,
        AnimationData& aniData) {
        Vector3 vmin(FLT_MAX, FLT_MAX, FLT_MAX);
        Vector3 vmax(-FLT_MAX, -FLT_MAX, -FLT_MAX);
        for (auto& mesh : meshes) for (auto& v : mesh.vertices) {
            vmin = Vector3(
                std::min(vmin.x, v.position.x),
                std::min(vmin.y, v.position.y),
                std::min(vmin.z, v.position.z));
            vmax = Vector3(
                std::max(vmax.x, v.position.x),
                std::max(vmax.y, v.position.y),
                std::max(vmax.z, v.position.z));
        }
        float dx = vmax.x - vmin.x;
        float dy = vmax.y - vmin.y;
        float dz = vmax.z - vmin.z;
        float scale = longestLength / std::max({ dx, dy, dz });
        Vector3 trans = -(vmin + vmax) * 0.5f + center;
        for (auto& mesh : meshes) {
            for (auto& v : mesh.vertices) {
                v.position = (v.position + trans) * scale;
            }
            for (auto& sv : mesh.skinnedVertices) {
                sv.position = (sv.position + trans) * scale;
            }
        }
        aniData.defaultTransform =
            Matrix::CreateTranslation(trans) * Matrix::CreateScale(scale);
    }

} // namespace BinCore
