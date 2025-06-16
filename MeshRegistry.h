#pragma once
#include "Handle.h"
#include <unordered_map>
#include <memory>
#include <d3d11.h>

namespace BinRenderer {

    struct Mesh {
        ID3D11Buffer* vertexBuffer = nullptr;
        ID3D11Buffer* indexBuffer = nullptr;
        uint32_t indexCount = 0;
        UINT vertexStride = 0; // 정점 하나의 크기 (ex: sizeof(Vertex))
        UINT vertexOffset = 0; // 버퍼 내 정점 시작 위치 (보통 0)
    };

    class MeshRegistry {
    public:
        MeshHandle Register(const Mesh& mesh);
        const Mesh* Get(MeshHandle handle) const;

    private:
        std::unordered_map<uint16_t, Mesh> m_meshes;
        uint16_t m_nextId = 0;
    };
}
