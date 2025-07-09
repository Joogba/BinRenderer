#pragma once

#include "Core/RenderEnums.h"

#include <cstdint>

namespace BinRenderer {

    // 입력 레이아웃 요소 설명
    struct InputElementDesc {
        const char* SemanticName;
        uint32_t    SemanticIndex;
        Format      Format;
        uint32_t    InputSlot;
        uint32_t    AlignedByteOffset;
        uint32_t    InputSlotClass;      // 0: per-vertex, 1: per-instance
        uint32_t    InstanceDataStepRate;
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
    };

    

    struct DepthStencilOpDesc {
        ComparisonFunc StencilFunc;
        uint8_t        StencilPassOp;
        uint8_t        StencilFailOp;
        uint8_t        StencilDepthFailOp;
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
    };

   


    struct RenderTargetBlendDesc {
        bool     BlendEnable;
        Blend    SrcBlend;
        Blend    DestBlend;
        BlendOp  BlendOp;
        Blend    SrcBlendAlpha;
        Blend    DestBlendAlpha;
        BlendOp  BlendOpAlpha;
        uint8_t  RenderTargetWriteMask;
    };

    struct BlendState {
        bool                    AlphaToCoverageEnable;
        bool                    IndependentBlendEnable;
        RenderTargetBlendDesc   RenderTarget[8];
    };

    


    struct SamplerDesc {
        FilterMode     filter;
        AddressMode    addressU;
        AddressMode    addressV;
        AddressMode    addressW;
        ComparisonFunc comparison;
        float          minLOD;
        float          maxLOD;
        float          mipLODBias;
        uint32_t       maxAnisotropy;
        float          borderColor[4];
    };

    // 텍스처 생성 파라미터
    struct TextureDesc {
        uint32_t width;
        uint32_t height;
        Format   format;
        uint32_t bindFlags;  // BindFlags 조합
    };

    // 파이프라인 상태 생성 파라미터
    struct PSODesc {
        std::string         name;
        std::string         vsFile;
        std::string         vsEntry;
        std::string         psFile;
        std::string         psEntry;
        std::vector<InputElementDesc> inputElements;
        RasterizerState     rasterizerState;
        DepthStencilState   depthStencilState;
        BlendState          blendState;
        float               blendFactor[4] = { 1,1,1,1 };
        uint32_t                stencilRef = 0;
        PrimitiveTopology           primitiveTopology = PrimitiveTopology::TriangleList;
        uint32_t                               sampleMask = 0xFFFFFFFF;

        uint32_t            numRenderTargets = 1;
        Format         rtvFormats[8];
        Format         dsvFormat;
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

    struct ShaderDesc {
        ShaderStage stage;
        std::string entryPoint;
        std::vector<uint8_t> byteCode; // 컴파일된 바이너리(플랫폼 별도)
        std::string name;
    };

} // namespace BinRenderer
