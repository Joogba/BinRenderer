#pragma once
#include "Core/Handle.h"
#include <unordered_map>
#include <memory>
#include <wrl/client.h>
#include <d3d11.h>

namespace BinRenderer {

using Microsoft::WRL::ComPtr;

    struct Mesh { // TODO : Api 의존성 제거
        ComPtr<ID3D11Buffer> vertexBuffer;
        ComPtr<ID3D11Buffer> indexBuffer;
        UINT indexCount = 0;
        UINT vertexStride = 0; // 정점 하나의 크기 (ex: sizeof(Vertex))
        UINT vertexOffset = 0; // 버퍼 내 정점 시작 위치 (보통 0)

        // 인스턴스 데이터 스트림
        ComPtr<ID3D11Buffer> instanceBuffer;
        UINT                 instanceStride = 0;
        UINT                 instanceOffset = 0;
    };

    class MeshRegistry {
    public:
        MeshHandle Register(const std::string& name, const Mesh& mesh) {
            auto it = m_nameToIdx.find(name);
            if (it != m_nameToIdx.end()) return MeshHandle(it->second);
            MeshHandle handle(m_nextId++);
            m_meshes.emplace(handle.idx, mesh);
            m_nameToIdx[name] = handle.idx;
            m_idxToName[handle.idx] = name;
            return handle;
        }

        const Mesh* Get(MeshHandle handle) const {
            auto it = m_meshes.find(handle.idx);
            return it != m_meshes.end() ? &it->second : nullptr;
        }

        const Mesh* Get(const std::string& name) const {
            auto it = m_nameToIdx.find(name);
            return it != m_nameToIdx.end() ? Get(MeshHandle(it->second)) : nullptr;
        }

        MeshHandle GetHandle(const std::string& name) const {
            auto it = m_nameToIdx.find(name);
            return it != m_nameToIdx.end() ? MeshHandle(it->second) : MeshHandle();
        }

        const std::string& GetName(MeshHandle handle) const {
            static std::string empty;
            auto it = m_idxToName.find(handle.idx);
            return it != m_idxToName.end() ? it->second : empty;
        }


    private:
        std::unordered_map<uint16_t, Mesh> m_meshes;
        std::unordered_map<std::string, uint16_t> m_nameToIdx;
        std::unordered_map<uint16_t, std::string> m_idxToName;
        uint16_t m_nextId = 1;
    };
}
