#pragma once

#include "Core/Handle.h"
#include <unordered_map>
#include <string>

#include "PSORegistry.h"
#include "UniformSystem.h"

#include <memory>

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

         // Texture / Sampler 등록 API
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
        MaterialHandle Register(const std::string& name, const Material& mat) {
            auto it = m_nameToIdx.find(name);
            if (it != m_nameToIdx.end()) return MaterialHandle(it->second);
            MaterialHandle handle(m_nextId++);
            m_materials.emplace(handle.idx, mat);
            m_nameToIdx[name] = handle.idx;
            m_idxToName[handle.idx] = name;
            return handle;
        }

        const Material* Get(MaterialHandle handle) const {
            auto it = m_materials.find(handle.idx);
            return it != m_materials.end() ? &it->second : nullptr;
        }

        const Material* Get(const std::string& name) const {
            auto it = m_nameToIdx.find(name);
            return it != m_nameToIdx.end() ? Get(MaterialHandle(it->second)) : nullptr;
        }

        MaterialHandle GetHandle(const std::string& name) const {
            auto it = m_nameToIdx.find(name);
            return it != m_nameToIdx.end() ? MaterialHandle(it->second) : MaterialHandle();
        }

        const std::string& GetName(MaterialHandle handle) const {
            static std::string empty;
            auto it = m_idxToName.find(handle.idx);
            return it != m_idxToName.end() ? it->second : empty;
        }

    private:
        std::unordered_map<uint16_t, Material> m_materials;
        std::unordered_map<std::string, uint16_t> m_nameToIdx;
        std::unordered_map<uint16_t, std::string> m_idxToName;
        uint16_t m_nextId = 1;
    };

} // namespace BinRenderer
