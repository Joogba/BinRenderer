#pragma once
#include "Handle.h"
#include <unordered_map>
#include <memory>
#include <wrl/client.h>
#include <d3d11.h>

namespace BinRenderer {

using Microsoft::WRL::ComPtr;

    struct Mesh {
        ComPtr<ID3D11Buffer> vertexBuffer;
        ComPtr<ID3D11Buffer> indexBuffer;
        UINT indexCount = 0;
        UINT vertexStride = 0; // ���� �ϳ��� ũ�� (ex: sizeof(Vertex))
        UINT vertexOffset = 0; // ���� �� ���� ���� ��ġ (���� 0)

        // �ν��Ͻ� ������ ��Ʈ��
        ComPtr<ID3D11Buffer> instanceBuffer;
        UINT                 instanceStride = 0;
        UINT                 instanceOffset = 0;
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
