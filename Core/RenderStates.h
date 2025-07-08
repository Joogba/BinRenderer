#pragma once

#include "Format.h"
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

} // namespace BinRenderer
