#pragma once
#include <cstdint>
#include <functional>

namespace BinRenderer {

    template<typename Tag, typename T = uint16_t>
    struct Handle {
        T idx;

        static constexpr T Invalid = UINT16_MAX;

        constexpr Handle() : idx(Invalid) {}
        constexpr explicit Handle(T _idx) : idx(_idx) {}
        constexpr bool isValid() const { return idx != Invalid; }

        constexpr operator T() const { return idx; }

        // Comparison operators for use in containers
        constexpr bool operator==(const Handle& other) const { return idx == other.idx; }
        constexpr bool operator!=(const Handle& other) const { return idx != other.idx; }
        constexpr bool operator<(const Handle& other) const { return idx < other.idx; }
    };

    struct MeshTag {};
    struct MaterialTag {};
    struct PSOTag {};
    struct TextureHandleTag {};
    struct RTVHandleTag {};
    struct SRVHandleTag {};
    struct DSVHandleTag {};
    struct SamplerHandleTag {};
    struct ShaderHandleTag {};

    using MeshHandle = Handle<MeshTag>;
    using MaterialHandle = Handle<MaterialTag>;
    using PSOHandle = Handle<PSOTag>;
    using TextureHandle = Handle<TextureHandleTag>;
    using RenderTargetViewHandle = Handle<RTVHandleTag>;
    using ShaderResourceViewHandle = Handle<SRVHandleTag>;
    using DepthStencilViewHandle = Handle<DSVHandleTag>;
    using SamplerHandle = Handle<SamplerHandleTag>;
    using ShaderHandle = Handle<ShaderHandleTag>;

} // namespace BinRenderer

// std::hash specialization for Handle types
namespace std {
    template<typename Tag, typename T>
    struct hash<BinRenderer::Handle<Tag, T>> {
        size_t operator()(const BinRenderer::Handle<Tag, T>& handle) const noexcept {
            return std::hash<T>{}(handle.idx);
        }
    };
}
