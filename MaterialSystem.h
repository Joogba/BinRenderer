#pragma once

#include "Handle.h"
#include "PSORegistry.h"
#include "UniformSystem.h"

#include <memory>
#include <d3d11.h>

namespace BinRenderer {

    struct TextureBinding {
        uint32_t        slot;
        TextureHandle   handle;
    };
    struct SamplerBinding {
        uint32_t        slot;
        SamplerHandle   handle;
    };

    class Material {
    public:
        Material(PSOHandle pso, std::shared_ptr<UniformLayout> layout);

        PSOHandle GetPSO() const { return m_pso; }
        UniformSet& GetUniformSet() { return *m_uniformSet; }
        const UniformSet& GetUniformSet() const { return *m_uniformSet; }

         // Texture / Sampler µî·Ï API
         void BindTexture(uint32_t slot, TextureHandle th) { m_textures.push_back({ slot, th }); }
         void BindSampler(uint32_t slot, SamplerHandle sh) { m_samplers.push_back({ slot, sh }); }

         const std::vector<TextureBinding>&GetTextureBindings() const { return m_textures; }
         const std::vector<SamplerBinding>&GetSamplerBindings() const { return m_samplers; }

    private:
        PSOHandle m_pso;
        std::unique_ptr<UniformSet> m_uniformSet;

        std::vector<TextureBinding> m_textures;
        std::vector<SamplerBinding> m_samplers;
    };

    class MaterialRegistry {
    public:
        MaterialHandle Register(std::unique_ptr<Material> material);
        Material* Get(MaterialHandle handle);
        const Material* Get(MaterialHandle handle) const;

    private:
        std::unordered_map<uint16_t, std::unique_ptr<Material>> m_materials;
        uint16_t m_nextId = 0;
    };

} // namespace BinRenderer
