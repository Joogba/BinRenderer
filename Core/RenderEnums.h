#pragma once
#include <cstdint>

namespace BinRenderer {

    enum class ShaderStage {
        Vertex,
        Pixel,
        Compute,
        Geometry,
        Hull,
        Domain,
    };
    // Clear 플래그 (비트마스크)
    enum class ClearFlags : uint32_t {
        ClearColor = 1 << 0,
        ClearDepth = 1 << 1,
        ClearStencil = 1 << 2,
    };

    // 텍스처 포맷
    enum class Format {
        RGBA32_FLOAT,
        R8G8B8A8_UNORM,
        R16G16B16A16_FLOAT,
        R32_FLOAT,
        DEPTH24_STENCIL8,
        Unknown,
        // ... 필요시 추가
    };

    // 바인드 플래그
    enum class BindFlags : uint32_t {
        Bind_None = 0,
        Bind_RenderTarget = 1 << 0,
        Bind_ShaderResource = 1 << 1,
        Bind_DepthStencil = 1 << 2,
    };

    // API 타입 확장 대비
    enum class APIType { None, D3D11, Vulkan, OpenGL };

    

    // Primitive Topology
    enum class PrimitiveTopology {
        Undefined,
        TriangleList,
        TriangleStrip,
        LineList,
        LineStrip,
        PointList,
        // ...
    };

    // Blend Mode
    enum class Blend {
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
        // ...
    };
    enum class BlendOp {
        Add = 1,
        Subtract = 2,
        RevSubtract = 3,
        Min = 4,
        Max = 5
    };

    // Depth/Stencil
    enum class DepthFunc {
        Never,
        Less,
        Equal,
        LessEqual,
        Greater,
        NotEqual,
        // ...
    };
    enum class StencilOp {
        Keep,
        Zero,
        Replace,
        // ...
    };

    // Cull Mode
    enum class CullMode {
        None,
        Front,
        Back,
    };


    // Sampler
    enum class FilterMode {
        Point,
        Linear,
        Anisotropic,
    };
    enum class AddressMode {
        Wrap = 1,
        Mirror = 2,
        Clamp = 3,
        Border = 4,
        MirrorOnce = 5
    };

    // 깊이-스텐실 상태
    enum class ComparisonFunc {
        Never = 1,
        Less = 2,
        Equal = 3,
        LessEqual = 4,
        Greater = 5,
        NotEqual = 6,
        GreaterEqual = 7,
        Always = 8
    };

    // 래스터라이저 상태
    enum class FillMode {
        Wireframe = 2,
        Solid = 3,
    };

}