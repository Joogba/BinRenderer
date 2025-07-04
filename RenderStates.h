#pragma once

#include <cstdint>
#include <dxgiformat.h>  // For DXGI_FORMAT

namespace BinRenderer {

    // 입력 레이아웃 요소 설명
    struct InputElementDesc {
        const char* SemanticName;
        uint32_t    SemanticIndex;
        DXGI_FORMAT Format;
        uint32_t    InputSlot;
        uint32_t    AlignedByteOffset;
        uint32_t    InputSlotClass;      // 0: per-vertex, 1: per-instance
        uint32_t    InstanceDataStepRate;
    };

    // 래스터라이저 상태
    enum class FillMode : uint32_t {
        Wireframe = 2,
        Solid = 3
    };

    enum class CullMode : uint32_t {
        None = 1,
        Front = 2,
        Back = 3
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

    // 깊이-스텐실 상태
    enum class ComparisonFunc : uint32_t {
        Never = 1,
        Less = 2,
        Equal = 3,
        LessEqual = 4,
        Greater = 5,
        NotEqual = 6,
        GreaterEqual = 7,
        Always = 8
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

    // 블렌드 상태
    enum class Blend : uint32_t {
        Zero = 1,
        One = 2,
        SrcColor = 3,
        InvSrcColor = 4,
        SrcAlpha = 5,
        InvSrcAlpha = 6,
        DestAlpha = 7,
        InvDestAlpha = 8,
        DestColor = 9,
        InvDestColor = 10,
        BlendFactor = 14,
        InvBlendFactor = 15
    };

    enum class BlendOp : uint32_t {
        Add = 1,
        Subtract = 2,
        RevSubtract = 3,
        Min = 4,
        Max = 5
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

    // 샘플러 상태
    enum class FilterMode : uint32_t {
        Point = 0,
        Linear = 1,
        Anisotropic = 2
    };

    enum class AddressMode : uint32_t {
        Wrap = 1,
        Mirror = 2,
        Clamp = 3,
        Border = 4,
        MirrorOnce = 5
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
