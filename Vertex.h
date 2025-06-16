#pragma once

#include <directxtk/SimpleMath.h>
#include <vector>

namespace BinRenderer {

    using DirectX::SimpleMath::Vector2;
    using DirectX::SimpleMath::Vector3;
    using DirectX::SimpleMath::Vector4;

    struct Vertex {
        Vector3 position;
        Vector3 normalModel;
        Vector2 texcoord;
        Vector3 tangentModel;
        // Vector3 biTangentModel; // biTangent�� ���̴����� ���
    };

    struct SkinnedVertex {
        Vector3 position;
        Vector3 normalModel;
        Vector2 texcoord;
        Vector3 tangentModel;

        float blendWeights[8] = { 0.0f, 0.0f, 0.0f, 0.0f,
                                 0.0f, 0.0f, 0.0f, 0.0f };  // BLENDWEIGHT0 and 1
        uint8_t boneIndices[8] = { 0, 0, 0, 0, 0, 0, 0, 0 }; // BLENDINDICES0 and 1

        // boneWeights�� �ִ� 8����� ���� (Luna ���翡���� 4��)
        // bone�� ���� 256�� ���϶�� ���� uint8_t
    };

    struct GrassVertex {
        Vector3 posModel;
        Vector3 normalModel;
        Vector2 texcoord;

        // ����: Instance World�� ������ ���۷� ����
    };

    // GrassVS, grassIL�� �ϰ����� �־�� �մϴ�.
    struct GrassInstance {
        DirectX::SimpleMath::Matrix instanceWorld; // <- Instance ������ Model to World ��ȯ
        float windStrength;
    };

} // namespace BinCore