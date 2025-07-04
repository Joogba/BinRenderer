#pragma once
#include <cstdint>

namespace BinRenderer {

    template<typename Tag, typename T = uint16_t>
    struct Handle {
        T idx;

        static constexpr T Invalid = UINT16_MAX;

        constexpr Handle() : idx(Invalid) {}
        constexpr explicit Handle(T _idx) : idx(_idx) {}
        constexpr bool isValid() const { return idx != Invalid; }

        constexpr operator T() const { return idx; }
    };

    struct MeshTag {};
    struct MaterialTag {};
    struct PSOTag {};
    struct TextureHandleTag {};
    struct RTVHandleTag {};
    struct SRVHandleTag {};
    struct DSVHandleTag {};

    using MeshHandle = Handle<MeshTag>;
    using MaterialHandle = Handle<MaterialTag>;
    using PSOHandle = Handle<PSOTag>;
    using TextureHandle = Handle<TextureHandleTag>;
    using RenderTargetViewHandle = Handle<RTVHandleTag>;
    using ShaderResourceViewHandle = Handle<SRVHandleTag>;
    using DepthStencilViewHandle = Handle<DSVHandleTag>;

    struct TextureHandle { uint32_t idx; };
    struct SamplerHandle { uint32_t idx; };

} // namespace BinRenderer
