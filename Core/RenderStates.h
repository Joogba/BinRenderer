#pragma once

#include "Core/RenderEnums.h"
#include "Core/Handle.h"

#include <cstdint>
#include <string>
#include <array>
#include <vector>

namespace BinRenderer {

    // 입력 레이아웃 요소 설명
    struct InputElementDesc {
        const char* SemanticName;
        uint32_t    SemanticIndex;
        Format      format;
        uint32_t    InputSlot;
        uint32_t    AlignedByteOffset;
        uint32_t    InputSlotClass;      // 0: per-vertex, 1: per-instance
        uint32_t    InstanceDataStepRate;
        
        // 비교 연산자 추가
        bool operator==(const InputElementDesc& other) const {
            return SemanticIndex == other.SemanticIndex
                && format == other.format
                && InputSlot == other.InputSlot
                && AlignedByteOffset == other.AlignedByteOffset
                && InputSlotClass == other.InputSlotClass
                && InstanceDataStepRate == other.InstanceDataStepRate
                && strcmp(SemanticName, other.SemanticName) == 0;
        }
    };

    

    struct RasterizerState {
        FillMode fillMode;
        CullMode cullMode;
        bool     frontCounterClockwise;
        int      depthBias;
        float    depthBiasClamp;
        float    slopeScaledDepthBias;
        bool     depthClipEnable;
        bool     scissorEnable;
        bool     multisampleEnable;
        bool operator==(const RasterizerState& other) const {
            return fillMode == other.fillMode &&
                cullMode == other.cullMode &&
                frontCounterClockwise == other.frontCounterClockwise &&
                depthBias == other.depthBias &&
                depthBiasClamp == other.depthBiasClamp &&
                slopeScaledDepthBias == other.slopeScaledDepthBias &&
                depthClipEnable == other.depthClipEnable &&
                scissorEnable == other.scissorEnable &&
                multisampleEnable == other.multisampleEnable;
        }
    };

    

    struct DepthStencilOpDesc {
        ComparisonFunc StencilFunc;
        StencilOp        StencilPassOp;
        StencilOp        StencilFailOp;
        StencilOp        StencilDepthFailOp;

        bool operator==(const DepthStencilOpDesc& other) const {
            return StencilFunc == other.StencilFunc &&
                   StencilPassOp == other.StencilPassOp &&
                   StencilFailOp == other.StencilFailOp &&
                   StencilDepthFailOp == other.StencilDepthFailOp;
        }
    };

    struct DepthStencilState {
        bool                DepthEnable;
        bool                DepthWriteMask;
        ComparisonFunc      DepthFunc;

        bool                StencilEnable;
        uint8_t             StencilReadMask;
        uint8_t             StencilWriteMask;
        DepthStencilOpDesc  FrontFace;
        DepthStencilOpDesc  BackFace;

        bool operator==(const DepthStencilState& other) const {
            return DepthEnable == other.DepthEnable &&
                   DepthWriteMask == other.DepthWriteMask &&
                   DepthFunc == other.DepthFunc &&
                   StencilEnable == other.StencilEnable &&
                   StencilReadMask == other.StencilReadMask &&
                   StencilWriteMask == other.StencilWriteMask &&
                   FrontFace == other.FrontFace &&
                   BackFace == other.BackFace;
        }
    };

   


    struct RenderTargetBlendDesc {
        bool     blendEnable;
        Blend    srcBlend;
        Blend    destBlend;
        BlendOp  blendOp;
        Blend    srcBlendAlpha;
        Blend    destBlendAlpha;
        BlendOp  blendOpAlpha;
        uint8_t  renderTargetWriteMask;
        bool operator==(const RenderTargetBlendDesc& other) const {
            return blendEnable == other.blendEnable &&
                srcBlend == other.srcBlend &&
                destBlend == other.destBlend &&
                blendOp == other.blendOp &&
                srcBlendAlpha == other.srcBlendAlpha &&
                destBlendAlpha == other.destBlendAlpha &&
                blendOpAlpha == other.blendOpAlpha &&
                renderTargetWriteMask == other.renderTargetWriteMask;
        }
    };

    struct BlendState {
        bool                    AlphaToCoverageEnable;
        bool                    IndependentBlendEnable;
        RenderTargetBlendDesc   RenderTarget[8];
    };

    


   

    // 샘플러 생성 파라미터
    struct SamplerDesc {
        FilterMode     filter;
        AddressMode    addressU;
        AddressMode    addressV;
        AddressMode    addressW;
        ComparisonFunc comparison;
        float          minLOD;
        float          maxLOD;
        float          mipLODBias;       // LOD 바이어스 (기본 0.0f)
        uint32_t       maxAnisotropy;    // 최대 이방성 필터링 레벨 (기본 1)
        float          borderColor[4];   // 보더 컬러 (Wrap=Border 모드 시 사용)
    };

    // 텍스처 생성 파라미터
    struct TextureDesc {
        uint32_t width;
        uint32_t height;
        Format   format;
        uint32_t bindFlags;  // BindFlags 조합

        // 비교 연산자 추가
        bool operator==(const TextureDesc& other) const {
            return width == other.width
                && height == other.height
                && format == other.format
                && bindFlags == other.bindFlags;
        }
    };

    // 파이프라인 상태 생성 파라미터
    struct PSODesc
    {
        // 셰이더 핸들(플랫폼 독립!)
        ShaderHandle vertexShader;
        ShaderHandle pixelShader;
        ShaderHandle hullShader;
        ShaderHandle domainShader;
        ShaderHandle geometryShader;

        // 입력 레이아웃(플랫폼 독립 구조체)
        std::vector<InputElementDesc> inputLayout;

        // 상태 객체(플랫폼 독립 구조체)
        RenderTargetBlendDesc         blendState;
        DepthStencilState  depthStencilState;
        RasterizerState    rasterizerState;

        // 파라미터(플랫폼 독립)
        std::array<float,4> blendFactor = { 1.0f, 1.0f, 1.0f, 1.0f };
        uint32_t            stencilRef = 0;
        PrimitiveTopology   primitiveTopology = PrimitiveTopology::TriangleList;
        uint32_t            sampleMask = 0xFFFFFFFF;

        // 필요한 경우: color/depth 스펙, 멀티렌더타겟 등
        std::array<Format, 8> rtvFormats = { Format::Unknown };
        Format               dsvFormat = Format::Unknown;
        uint32_t             numRenderTargets = 1;
        // 기타 확장 필요시 필드 추가!
    };

    // 해시 결합 유틸리티
    inline void HashCombine(size_t& seed, size_t value) {
        seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    struct PSODescHash {
        size_t operator()(const PSODesc& desc) const {
            size_t h = 0;
            // 셰이더 핸들 해시
            HashCombine(h, std::hash<uint32_t>()(static_cast<uint32_t>(desc.vertexShader)));
            HashCombine(h, std::hash<uint32_t>()(static_cast<uint32_t>(desc.pixelShader)));
            HashCombine(h, std::hash<uint32_t>()(static_cast<uint32_t>(desc.hullShader)));
            HashCombine(h, std::hash<uint32_t>()(static_cast<uint32_t>(desc.domainShader)));
            HashCombine(h, std::hash<uint32_t>()(static_cast<uint32_t>(desc.geometryShader)));
            // 입력 레이아웃
            for (const auto& elem : desc.inputLayout) {
                size_t elemHash = std::hash<std::string>()(elem.SemanticName);
                HashCombine(elemHash, std::hash<uint32_t>()(elem.SemanticIndex));
                HashCombine(elemHash, std::hash<int>()(static_cast<int>(elem.format)));
                HashCombine(elemHash, std::hash<uint32_t>()(elem.InputSlot));
                HashCombine(elemHash, std::hash<uint32_t>()(elem.AlignedByteOffset));
                HashCombine(elemHash, std::hash<uint32_t>()(elem.InputSlotClass));
                HashCombine(elemHash, std::hash<uint32_t>()(elem.InstanceDataStepRate));
                HashCombine(h, elemHash);
            }
            // 상태 해시
            HashCombine(h, std::hash<int>()(static_cast<int>(desc.blendState.blendOp)));
            HashCombine(h, std::hash<int>()(static_cast<int>(desc.depthStencilState.DepthFunc)));
            HashCombine(h, std::hash<int>()(static_cast<int>(desc.rasterizerState.cullMode)));

            // BlendFactor
            for (auto v : desc.blendFactor)
                HashCombine(h, std::hash<float>()(v));
            HashCombine(h, std::hash<uint32_t>()(desc.stencilRef));
            HashCombine(h, std::hash<int>()(static_cast<int>(desc.primitiveTopology)));
            HashCombine(h, std::hash<uint32_t>()(desc.sampleMask));

            // RenderTargetFormats
            for (auto f : desc.rtvFormats)
                HashCombine(h, std::hash<int>()(static_cast<int>(f)));
            HashCombine(h, std::hash<int>()(static_cast<int>(desc.dsvFormat)));
            HashCombine(h, std::hash<uint32_t>()(desc.numRenderTargets));
            return h;
        }
    };

   

    struct ShaderDesc {
        ShaderStage stage;
		std::string filePath; // 쉐이더 파일 경로
        std::string entryPoint;
		std::string profile; // 예: "vs_5_0", "ps_5_0"
        std::string name;
    };

 //   struct MaterialDesc {
 //       std::string name;
 //       PSOHandle psoHandle; // 파이프라인 상태 오브젝트 핸들
 //       std::vector<TextureHandle> textureBindings; // 텍스처 바인딩
 //       std::vector<TextureHandle> samplerBindings; // 샘플러 바인딩
 //       UniformSet uniformSet; // 유니폼 세트
	//};

} // namespace BinRenderer
