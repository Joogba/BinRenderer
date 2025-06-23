Texture2D    g_tex0 : register(t0);
SamplerState g_smp0 : register(s0);

cbuffer ObjectData : register(b0)
{
    matrix modelMatrix;  // Set("modelMatrix", ...)
    matrix viewProj;     // Set("viewProj"   , ...)
};

struct VSInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
    float3 Tangent : TANGENT;

    // 인스턴스 행렬을 4개의 float4로 받는다
    float4 InstMatrix0 : TEXCOORD1;
    float4 InstMatrix1 : TEXCOORD2;
    float4 InstMatrix2 : TEXCOORD3;
    float4 InstMatrix3 : TEXCOORD4;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
};

VSOutput VSMain(VSInput input)
{
    VSOutput o;
    // 1) 모델 공간 → 월드 공간
    float4x4 instanceMatrix = float4x4(
        input.InstMatrix0,
        input.InstMatrix1,
        input.InstMatrix2,
        input.InstMatrix3
    );
    float4 worldPos = mul(float4(input.Position, 1), instanceMatrix);
    // 2) 월드 공간 → 클립 공간 (뷰·투영 적용)
    o.Position = mul(worldPos, viewProj);
    o.UV = input.TexCoord;
    return o;
}

float4 PSMain(VSOutput input) : SV_TARGET
{
   
    float4 color = g_tex0.Sample(g_smp0, input.UV);
    return color;
}
