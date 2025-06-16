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
        UINT vertexStride = 0; // ���� �ϳ��� ũ�� (ex: sizeof(Vertex))
        UINT vertexOffset = 0; // ���� �� ���� ���� ��ġ (���� 0)
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
