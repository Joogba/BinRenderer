#version 450

// Vertex input attributes matching the updated Vertex class layout
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;
layout(location = 5) in vec4 inBoneWeights;
layout(location = 6) in ivec4 inBoneIndices;

layout(set = 0, binding = 0) uniform SceneDataUBO {
    mat4 projection;
    mat4 view;
    vec3 cameraPos;
    float padding1;
    vec3 directionalLightDir;
    float padding2;
    vec3 directionalLightColor;
    float padding3;
    mat4 lightSpaceMatrix;
} sceneData;

layout(set = 0, binding = 1) uniform OptionsUBO {
    bool textureOn;
    bool shadowOn;
    bool discardOn;
    bool animationOn;
    float ssaoRadius;
    float ssaoBias;
    int ssaoSampleCount;
    float ssaoPower;
} options;

layout(set = 0, binding = 2) uniform BoneDataUBO {
    mat4 boneMatrices[65];
    vec4 animationData;
} boneData;

// ? NEW: Instance transforms (for GPU instancing)
layout(set = 0, binding = 3) readonly buffer InstanceTransforms {
    mat4 instanceMatrices[];
} instanceTransforms;

layout(push_constant) uniform PushConstants {
    mat4 model;              // ?? 인스턴싱 사용 시 무시됨
    uint materialIndex;
    float coeffs[15];
} pushConstants;

// Output to fragment shader
layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out vec3 fragTangent;
layout(location = 4) out vec3 fragBitangent;
layout(location = 5) out vec3 fragCameraPos;
layout(location = 6) out vec4 fragPosLightSpace;

void main() {
    vec3 position = inPosition;
    vec3 normal = inNormal;
    vec3 tangent = inTangent;
    vec3 bitangent = inBitangent;
    
    bool animationApplied = false;
    
    // ? Apply skeletal animation if enabled
    if (options.animationOn && boneData.animationData.x > 0.5) {
        mat4 boneTransform = mat4(0.0);
        float totalWeight = 0.0;
        
        for (int i = 0; i < 4; i++) {
            int boneIndex = inBoneIndices[i];
            float weight = inBoneWeights[i];
            
            if (boneIndex >= 0 && boneIndex < 65 && weight > 0.001) {
                boneTransform += boneData.boneMatrices[boneIndex] * weight;
                totalWeight += weight;
            }
        }
        
        if (totalWeight > 0.001) {
            vec4 animatedPosition = boneTransform * vec4(position, 1.0);
            position = animatedPosition.xyz;
            normal = mat3(boneTransform) * normal;
            tangent = mat3(boneTransform) * tangent;
            bitangent = mat3(boneTransform) * bitangent;
            animationApplied = true;
        }
    }
    
    // ? Select model matrix: use instance transform if available
    mat4 modelMatrix;
    if (gl_InstanceIndex > 0) {
        // GPU 인스턴싱 모드: SSBO에서 transform 가져오기
        modelMatrix = instanceTransforms.instanceMatrices[gl_InstanceIndex];
    } else {
        // 기존 모드: Push constant 사용
        modelMatrix = pushConstants.model;
    }
    
    // Transform position and normals
    vec4 worldPos = modelMatrix * vec4(position, 1.0);
    fragPos = worldPos.xyz;
    
    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
    fragNormal = normalize(normalMatrix * normal);
    fragTangent = normalize(normalMatrix * tangent);
    fragBitangent = normalize(normalMatrix * bitangent);
    
    fragTexCoord = inTexCoord;
    fragCameraPos = sceneData.cameraPos;
    fragPosLightSpace = sceneData.lightSpaceMatrix * worldPos;
    
    gl_Position = sceneData.projection * sceneData.view * worldPos;
}