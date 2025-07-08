#pragma once

#include <d3d11.h>
#include <memory>
#include <DirectXMath.h>

#include "MeshRegistry.h"
#include "MeshData.h"
#include "Vertex.h"

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

        static Mesh CreateCube(ID3D11Device* device, float size = 1.0f) {
            struct SimpleVertex {
                DirectX::XMFLOAT3 pos;
                DirectX::XMFLOAT3 normal;
                DirectX::XMFLOAT2 uv;
            };
            float h = size * 0.5f;
            SimpleVertex verts[] = {
                // +X
                {{+h,-h,-h},{1,0,0},{0,1}},
                {{+h,+h,-h},{1,0,0},{0,0}},
                {{+h,+h,+h},{1,0,0},{1,0}},
                {{+h,-h,+h},{1,0,0},{1,1}},
                // -X
                {{-h,-h,+h},{-1,0,0},{0,1}},
                {{-h,+h,+h},{-1,0,0},{0,0}},
                {{-h,+h,-h},{-1,0,0},{1,0}},
                {{-h,-h,-h},{-1,0,0},{1,1}},
                // +Y
                {{-h,+h,-h},{0,1,0},{0,1}},
                {{-h,+h,+h},{0,1,0},{0,0}},
                {{+h,+h,+h},{0,1,0},{1,0}},
                {{+h,+h,-h},{0,1,0},{1,1}},
                // -Y
                {{-h,-h,+h},{0,-1,0},{0,1}},
                {{-h,-h,-h},{0,-1,0},{0,0}},
                {{+h,-h,-h},{0,-1,0},{1,0}},
                {{+h,-h,+h},{0,-1,0},{1,1}},
                // +Z
                {{-h,-h,+h},{0,0,1},{0,1}},
                {{+h,-h,+h},{0,0,1},{1,1}},
                {{+h,+h,+h},{0,0,1},{1,0}},
                {{-h,+h,+h},{0,0,1},{0,0}},
                // -Z
                {{+h,-h,-h},{0,0,-1},{0,1}},
                {{-h,-h,-h},{0,0,-1},{1,1}},
                {{-h,+h,-h},{0,0,-1},{1,0}},
                {{+h,+h,-h},{0,0,-1},{0,0}},
            };
            uint16_t inds[] = {
                0,1,2, 0,2,3,      4,5,6, 4,6,7,
                8,9,10,8,10,11,   12,13,14,12,14,15,
                16,17,18,16,18,19,20,21,22,20,22,23
            };
            D3D11_BUFFER_DESC bd = {};
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            bd.ByteWidth = sizeof(verts);
            D3D11_SUBRESOURCE_DATA sd = { verts, 0,0 };
            Microsoft::WRL::ComPtr<ID3D11Buffer> vb;
            device->CreateBuffer(&bd, &sd, &vb);

            bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
            bd.ByteWidth = sizeof(inds);
            sd.pSysMem = inds;
            Microsoft::WRL::ComPtr<ID3D11Buffer> ib;
            device->CreateBuffer(&bd, &sd, &ib);

            Mesh m;
            m.vertexBuffer = vb.Get();
            m.indexBuffer = ib.Get();
            m.indexCount = _countof(inds);
            m.vertexStride = sizeof(SimpleVertex);
            m.vertexOffset = 0;
            return m;
        }

        static Mesh CreatePlane(ID3D11Device* device, float size = 10.0f) {
            struct Vert { DirectX::XMFLOAT3 p; DirectX::XMFLOAT3 n; DirectX::XMFLOAT2 uv; };
            float h = size * 0.5f;
            Vert verts[] = {
                {{-h,0,-h},{0,1,0},{0,1}},
                {{+h,0,-h},{0,1,0},{1,1}},
                {{+h,0,+h},{0,1,0},{1,0}},
                {{-h,0,+h},{0,1,0},{0,0}},
            };
            uint16_t inds[] = { 0,1,2, 0,2,3 };
            D3D11_BUFFER_DESC bd = {};
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            bd.ByteWidth = sizeof(verts);
            D3D11_SUBRESOURCE_DATA sd = { verts,0,0 };
            Microsoft::WRL::ComPtr<ID3D11Buffer> vb;
            device->CreateBuffer(&bd, &sd, &vb);

            bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
            bd.ByteWidth = sizeof(inds);
            sd.pSysMem = inds;
            Microsoft::WRL::ComPtr<ID3D11Buffer> ib;
            device->CreateBuffer(&bd, &sd, &ib);

            Mesh m;
            m.vertexBuffer = vb.Get();
            m.indexBuffer = ib.Get();
            m.indexCount = _countof(inds);
            m.vertexStride = sizeof(Vert);
            m.vertexOffset = 0;
            return m;
        }
    };

} // namespace BinRenderer
