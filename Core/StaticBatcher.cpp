#include "StaticBatcher.h"
#include <cassert>
#include <cstring>

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
        assert(mesh && "Invalid mesh handle");
    assert(mesh->vertexStride == sizeof(Vertex) && "Mesh vertex stride must match Vertex size");

    // 바이트 버퍼를 Vertex 배열로 해석
             const Vertex* vertices = reinterpret_cast<const Vertex*>(mesh->vertexBuffer.data());
 uint32_t vertexCount = mesh->vertexCount > 0 ? mesh->vertexCount : 
                  static_cast<uint32_t>(mesh->vertexBuffer.size() / mesh->vertexStride);

       // 각 정점에 transform을 적용해서 복사
      for (uint32_t i = 0; i < vertexCount; ++i)
     {
        Vertex vb = vertices[i];
           vb.position = glm::vec3(obj.world * glm::vec4(vb.position, 1.0f));
        vb.normalModel = glm::normalize(glm::mat3(obj.world) * vb.normalModel);
            vb.tangentModel = glm::normalize(glm::mat3(obj.world) * vb.tangentModel);
         batchedVertices.push_back(vb);
         }

      // 인덱스는 오프셋 보정해서 복사
        for (auto idx : mesh->indexBuffer)
   batchedIndices.push_back(idx + vertexOffset);

                vertexOffset += vertexCount;
    }

            // 3. 하나의 대형 메시로 MeshRegistry에 등록
         Mesh batchMesh;
        
     // Vertex 배열을 바이트 버퍼로 변환
            batchMesh.vertexBuffer.resize(batchedVertices.size() * sizeof(Vertex));
            std::memcpy(batchMesh.vertexBuffer.data(), batchedVertices.data(), 
  batchedVertices.size() * sizeof(Vertex));
            
       batchMesh.indexBuffer = std::move(batchedIndices);
            batchMesh.vertexCount = static_cast<uint32_t>(batchedVertices.size());
            batchMesh.indexCount = static_cast<uint32_t>(batchMesh.indexBuffer.size());
        batchMesh.vertexStride = sizeof(Vertex);
            batchMesh.meshType = MeshType::BatchedStatic;

        // 이름 자동생성 (Material 핸들 값 기반)
            std::string name = "StaticBatch_Mat" + std::to_string(static_cast<uint32_t>(material.idx));
       MeshHandle batchHandle = meshRegistry.Register(name, batchMesh);

 // 캐싱
 m_batchMeshes[material] = batchHandle;
        }
    }

}