#version 450

// ========================================
// ? GPU Instancing: Step 3 - Vertex Input
// ========================================

// Per-vertex attributes (기존)
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;
layout(location = 5) in vec4 inBoneWeights;   // Bone weights for skeletal animation
layout(location = 6) in ivec4 inBoneIndices;  // Bone indices for skeletal animation

// ? NEW: Per-instance attributes (from Instance Buffer)
// mat4는 4개의 vec4로 구성되므로 4개의 location을 차지함
layout(location = 10) in mat4 instanceModelMatrix;  // locations 10, 11, 12, 13
layout(location = 14) in uint instanceMaterialOffset;

// Uniform buffers
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
    bool isInstanced;  // ? NEW: GPU Instancing flag
} options;

layout(set = 0, binding = 2) uniform BoneDataUBO {
    mat4 boneMatrices[65];  // Support up to 65 bones (4,160 bytes)
    vec4 animationData;    // x = hasAnimation (0.0/1.0), y,z,w = future use
} boneData;

layout(push_constant) uniform PushConstants {
    mat4 model;  // ?? Legacy: 단일 인스턴스용 (instancing 시 무시됨)
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

    // Check animation flag from animationData.x
    bool hasAnimationEnabled = (boneData.animationData.x > 0.5);
    
    // Apply skeletal animation if enabled
    if (hasAnimationEnabled && (inBoneIndices.x >= 0 || inBoneIndices.y >= 0 || 
       inBoneIndices.z >= 0 || inBoneIndices.w >= 0)) {
  
        vec4 animatedPosition = vec4(0.0);
        vec3 animatedNormal = vec3(0.0);
        vec3 animatedTangent = vec3(0.0);
        vec3 animatedBitangent = vec3(0.0);
        
        for (int i = 0; i < 4; i++) {
            int boneIndex = inBoneIndices[i];
            float weight = inBoneWeights[i];
  
            if (boneIndex >= 0 && boneIndex < 65 && weight > 0.0) {
        mat4 boneMatrix = boneData.boneMatrices[boneIndex];
     
     animatedPosition += weight * (boneMatrix * vec4(inPosition, 1.0));
     
           mat3 boneNormalMatrix = mat3(boneMatrix);
  animatedNormal += weight * (boneNormalMatrix * inNormal);
          animatedTangent += weight * (boneNormalMatrix * inTangent);
          animatedBitangent += weight * (boneNormalMatrix * inBitangent);

      animationApplied = true;
       }
        }
     
        if (animatedPosition.w > 0.0) {
   position = animatedPosition.xyz;
      normal = normalize(animatedNormal);
      tangent = normalize(animatedTangent);
            bitangent = normalize(animatedBitangent);
    }
    }
  
    // DEBUG: Visual indicator for animation
    if (animationApplied && hasAnimationEnabled) {
 position.y += sin(gl_VertexIndex * 0.1) * 0.01;
    }

    // ========================================
    // ? GPU Instancing: Step 3 - Use Per-instance Transform
 // ========================================
    // ? NEW: 조건부 transform 선택
    mat4 modelMatrix;
  if (options.isInstanced) {
        // Instancing: per-instance transform 사용
        modelMatrix = instanceModelMatrix;
    } else {
        // Legacy: push constants transform 사용
      modelMatrix = pushConstants.model;
    }

    // Transform to world space
 vec4 worldPos = modelMatrix * vec4(position, 1.0);
fragPos = worldPos.xyz;
    
    const mat4 scaleBias = mat4(
        0.5, 0.0, 0.0, 0.0, 
        0.0, 0.5, 0.0, 0.0, 
        0.0, 0.0, 1.0, 0.0, 
   0.5, 0.5, 0.0, 1.0
    );

    // Shadow mapping
    fragPosLightSpace = scaleBias * sceneData.lightSpaceMatrix * worldPos;

  // Transform normals to world space
    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
    fragNormal = normalMatrix * normal;
    fragTangent = normalMatrix * tangent;
    fragBitangent = normalMatrix * bitangent;
    
    // Pass through
    fragTexCoord = inTexCoord;
    fragCameraPos = sceneData.cameraPos;
    
    // Final transform to clip space
    gl_Position = sceneData.projection * sceneData.view * worldPos;
}