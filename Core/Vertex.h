#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace BinRenderer {

    // DX 의존성 제거: glm::vec* 사용
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normalModel;
        glm::vec2 texcoord;
        glm::vec3 tangentModel;
        // glm::vec3 biTangentModel; // biTangent는 쉐이더에서 계산
    };

    struct SkinnedVertex {
        glm::vec3 position;
        glm::vec3 normalModel;
        glm::vec2 texcoord;
        glm::vec3 tangentModel;

        float blendWeights[8] = { 0.0f };
        uint8_t boneIndices[8] = { 0 };
        // boneWeights가 최대 8개라고 가정, bone 수가 256개 이하라고 가정
    };

    struct GrassVertex {
        glm::vec3 posModel;
        glm::vec3 normalModel;
        glm::vec2 texcoord;
        // Instance World는 별도 버퍼
    };

    // GrassVS, grassIL과 일관성이 있어야 합니다.
    struct alignas(16) GrassInstance {
        glm::mat4 instanceWorld; // glm::mat4로 변경
        float windStrength;
        float pad[3] = { 0.0f, 0.0f, 0.0f }; // 16바이트 정렬
    };

} // namespace BinRenderer