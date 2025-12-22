#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec4 inTangent;

layout(location = 0) out vec3 fragColor;

// ? Push Constants로 MVP 전달
layout(push_constant) uniform PushConstants {
    mat4 mvp;
} pc;

void main() {
    // ? MVP 변환 적용
    gl_Position = pc.mvp * vec4(inPosition, 1.0);
    
    // Normal을 컬러로 출력
    fragColor = inNormal * 0.5 + 0.5;
}
