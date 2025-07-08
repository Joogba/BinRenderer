#include "MeshFactory.h"
#include "GeometryGenerator.h"
#include "Core/Handle.h"
#include <cassert>
#include <wrl/client.h>




namespace BinRenderer {

    using Microsoft::WRL::ComPtr;


    std::unique_ptr<Mesh> MeshFactory::CreateMeshFromData(ID3D11Device* device, const MeshData& meshData) {
        auto mesh = std::make_unique<Mesh>();

        // 1) Vertex buffer
        const bool useSkinned = !meshData.skinnedVertices.empty();
        const void* verticesPtr = useSkinned
            ? static_cast<const void*>(meshData.skinnedVertices.data())
            : static_cast<const void*>(meshData.vertices.data());

        UINT vertexCount = useSkinned
            ? static_cast<UINT>(meshData.skinnedVertices.size())
            : static_cast<UINT>(meshData.vertices.size());
        mesh->vertexStride = useSkinned
            ? sizeof(SkinnedVertex)
            : sizeof(Vertex);
        mesh->vertexOffset = 0;

        D3D11_BUFFER_DESC vbDesc = {};
        vbDesc.ByteWidth = mesh->vertexStride * vertexCount;
        vbDesc.Usage = D3D11_USAGE_DEFAULT;
        vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA vbData = {};
        vbData.pSysMem = verticesPtr;
        HRESULT hr = device->CreateBuffer(&vbDesc, &vbData, &mesh->vertexBuffer);
        assert(SUCCEEDED(hr));

        // 2) Index buffer
        mesh->indexCount = static_cast<UINT>(meshData.indices.size());
        D3D11_BUFFER_DESC ibDesc = {};
        ibDesc.ByteWidth = sizeof(UINT) * mesh->indexCount;
        ibDesc.Usage = D3D11_USAGE_DEFAULT;
        ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA ibData = {};
        ibData.pSysMem = meshData.indices.data();
        hr = device->CreateBuffer(&ibDesc, &ibData, &mesh->indexBuffer);
        assert(SUCCEEDED(hr));

        return mesh;
    }

    std::unique_ptr<Mesh> MeshFactory::CreateQuadMesh(ID3D11Device* device, float scale) {
        // Use GeometryGenerator from Core layer
        auto quadData = GeometryGenerator::MakeSquare(scale, { 1.0f, 1.0f });
        return CreateMeshFromData(device, quadData);
    }

} // namespace BinRenderer