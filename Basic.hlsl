cbuffer ObjectData : register(b0)
{
    matrix modelMatrix;  // Set("modelMatrix", ...)
    matrix viewProj;     // Set("viewProj"   , ...)
};

struct VSInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
    float3 Tangent : TANGENT;
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
    float4 worldPos = mul(float4(input.Position, 1.0f), modelMatrix);
    // 2) 월드 공간 → 클립 공간 (뷰·투영 적용)
    o.Position = mul(worldPos, viewProj);
    o.UV = input.TexCoord;
    return o;
}

float4 PSMain(VSOutput i) : SV_TARGET
{
   
    return float4(i.UV, 0.0f, 1.0f);
}
