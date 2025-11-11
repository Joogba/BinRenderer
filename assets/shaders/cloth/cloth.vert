#version 450

// Cloth Vertex Shader: 간단 버전

// Storage Buffer에서 파티클 데이터 읽기
struct ClothParticle {
    vec4 position;   // xyz: 위치, w: invMass
    vec4 velocity;
    vec4 normal; // xyz: 노말, w: padding
};

layout(set = 0, binding = 0) readonly buffer PositionBuffer {
    ClothParticle particles[];
};

// Scene Uniform
layout(set = 0, binding = 1) uniform SceneUBO {
    mat4 viewProjection;
 vec3 cameraPos;
    float time;
} scene;

// Push Constants
layout(push_constant) uniform PushConstants {
    mat4 model;
} pc;

// Output
layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec3 fragWorldPos;
layout(location = 2) out vec2 fragTexCoord;

void main() {
    // 파티클 데이터 읽기
    ClothParticle particle = particles[gl_VertexIndex];
    
    vec3 position = particle.position.xyz;
    vec3 normal = particle.normal.xyz;
    
    // World space 변환
    vec4 worldPos = pc.model * vec4(position, 1.0);
 fragWorldPos = worldPos.xyz;
    
// Normal 변환
    mat3 normalMatrix = mat3(transpose(inverse(pc.model)));
    fragNormal = normalize(normalMatrix * normal);
  
    // UV 계산 (간단한 planar mapping)
    fragTexCoord = position.xz * 0.5 + 0.5;
    
    // Clip space
    gl_Position = scene.viewProjection * worldPos;
}
