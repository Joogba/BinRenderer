#include "StaticBatcher.h"
#include <cassert>

namespace BinRenderer {

    void StaticBatcher::AddStaticObject(const MeshHandle& mesh, const MaterialHandle& mat, const glm::mat4& transform) {
        m_pending.push_back({ mesh, mat, transform });
    }

    void StaticBatcher::Clear() {
        m_pending.clear();
        m_batchMeshes.clear();
    }

    void StaticBatcher::BuildBatches(MeshRegistry& meshRegistry)
    {
        m_batchMeshes.clear();

        // 1. 머티리얼별로 그룹핑
        std::unordered_map<MaterialHandle, std::vector<StaticObj>> groups;
        for (const auto& obj : m_pending)
            groups[obj.material].push_back(obj);

        // 2. 각 그룹별로 배칭 메시 생성
        for (auto& [material, objects] : groups)
        {
            // 최종 버텍스/인덱스 저장소
            std::vector<Vertex> batchedVertices;
            std::vector<uint32_t> batchedIndices;
            uint32_t vertexOffset = 0;

            for (const auto& obj : objects)
            {
                const Mesh* mesh = meshRegistry.Get(obj.mesh);
                assert(mesh);

                // 각 정점에 transform을 적용해서 복사
                for (const auto& v : mesh->vertexBuffer)
                {
                    Vertex vb = v;
                    vb.position = glm::vec3(obj.world * glm::vec4(vb.position, 1.0f));
                    if constexpr (std::is_member_object_pointer_v<decltype(Vertex::normalModel)>) {
                        vb.normalModel = glm::normalize(glm::mat3(obj.world) * vb.normalModel);
                    }
                    // tangent 등 필요시 transform
                    batchedVertices.push_back(vb);
                }

                // 인덱스는 오프셋 보정해서 복사
                for (auto idx : mesh->indexBuffer)
                    batchedIndices.push_back(idx + vertexOffset);

                vertexOffset += static_cast<uint32_t>(mesh->vertexBuffer.size());
            }

            // 3. 하나의 대형 메시로 MeshRegistry에 등록
            Mesh batchMesh;
            batchMesh.vertexBuffer = std::move(batchedVertices);
            batchMesh.indexBuffer = std::move(batchedIndices);
            batchMesh.indexCount = static_cast<uint32_t>(batchMesh.indexBuffer.size());
            batchMesh.vertexStride = sizeof(Vertex);

            // 이름 자동생성 (Material 핸들 값 기반)
            std::string name = "StaticBatch_" + std::to_string(static_cast<uint32_t>(material.idx));
            MeshHandle batchHandle = meshRegistry.Register(name, batchMesh);

            // 캐싱
            m_batchMeshes[material] = batchHandle;
        }
    }

}