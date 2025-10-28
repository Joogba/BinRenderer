#pragma once
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>

#include "Core/Vertex.h"
#include "Resources/MeshRegistry.h"
#include "Resources/MaterialRegistry.h" 

namespace BinRenderer {
    class StaticBatcher {
    public:
        struct StaticObj {
            MeshHandle mesh;
            MaterialHandle material;
            glm::mat4 world;
        };

        // 배칭할 정적 오브젝트 등록 (씬 초기화/로드 시)
        void AddStaticObject(const MeshHandle& mesh, const MaterialHandle& mat, const glm::mat4& transform);

        // 실제 배칭 메시 생성 (맵/씬 로딩 후 1회)
        void BuildBatches(MeshRegistry& meshRegistry);

        // 배칭 메시 반환(머티리얼별로)
        const std::unordered_map<MaterialHandle, MeshHandle>& GetBatchMeshes() const { return m_batchMeshes; }

        // 필요시: 클리어/재빌드
        void Clear();

    private:
        std::vector<StaticObj> m_pending; // 등록된 정적 오브젝트
        std::unordered_map<MaterialHandle, MeshHandle> m_batchMeshes; // 머티리얼별로 배치 메시 캐시
    };
}