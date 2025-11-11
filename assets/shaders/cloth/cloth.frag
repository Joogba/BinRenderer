#version 450

// Cloth Fragment Shader: 간단한 Phong 라이팅

// Input
layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec3 fragWorldPos;
layout(location = 2) in vec2 fragTexCoord;

// Output
layout(location = 0) out vec4 outColor;

// Scene Uniform
layout(set = 0, binding = 1) uniform SceneUBO {
    mat4 viewProjection;
    vec3 cameraPos;
    float time;
} scene;

void main() {
    // 기본 옷감 색상 (연한 회색)
    vec3 albedo = vec3(0.7, 0.7, 0.8);
    
    // 체커보드 패턴
    float checker = mod(floor(fragTexCoord.x * 16.0) + floor(fragTexCoord.y * 16.0), 2.0);
    albedo = mix(albedo, albedo * 0.5, checker * 0.3);
    
    // Light direction (고정)
    vec3 lightDir = normalize(vec3(0.5, -1.0, 0.3));
    vec3 lightColor = vec3(1.0, 0.95, 0.9);
    
    // Normal
    vec3 N = normalize(fragNormal);
    vec3 L = -lightDir;
    vec3 V = normalize(scene.cameraPos - fragWorldPos);
    vec3 H = normalize(L + V);
    
    // Ambient
    vec3 ambient = 0.4 * albedo;
    
    // Diffuse
    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse = NdotL * albedo * lightColor;
    
    // Specular
    float spec = pow(max(dot(N, H), 0.0), 16.0);
    vec3 specular = spec * vec3(0.2) * lightColor;
    
  // Two-sided lighting
    if (!gl_FrontFacing) {
   N = -N;
        NdotL = max(dot(N, L), 0.0);
      diffuse = NdotL * albedo * lightColor * 0.6;
        specular *= 0.3;
    }
    
  // Final color
    vec3 color = ambient + diffuse + specular;
    
    // Gamma correction
    color = pow(color, vec3(1.0 / 2.2));
    
    outColor = vec4(color, 1.0);
}
