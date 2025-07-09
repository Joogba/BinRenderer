#pragma once

#include "Core/Handle.h"
#include <unordered_map>
#include <string>

#include "UniformSystem.h"

#include <memory>

namespace BinRenderer {

    
    class Material {
    public:
        Material(PSOHandle pso, std::shared_ptr<UniformLayout> layout)
            : m_pso(pso), m_uniformSet(std::make_unique<UniformSet>(layout)) {
        }

        PSOHandle GetPSO() const { return m_pso; }
        UniformSet& GetUniformSet() { return *m_uniformSet; }
        const UniformSet& GetUniformSet() const { return *m_uniformSet; }

        // Texture / Sampler 등록 및 조회
        void BindTexture(uint32_t slot, TextureHandle th) { m_textures[slot] = th; }
        void BindSampler(uint32_t slot, SamplerHandle sh) { m_samplers[slot] = sh; }

        // 슬롯별 Texture/Sampler 핸들 조회
        TextureHandle GetTexture(uint32_t slot) const {
            auto it = m_textures.find(slot);
            return it != m_textures.end() ? it->second : TextureHandle();
        }
        SamplerHandle GetSampler(uint32_t slot) const {
            auto it = m_samplers.find(slot);
            return it != m_samplers.end() ? it->second : SamplerHandle();
        }

        // 바인딩 전체 조회 (렌더러에서 모두 순회 필요 시)
        const auto& GetTextureBindings() const { return m_textures; }
        const auto& GetSamplerBindings() const { return m_samplers; }

        // 슬롯 언바인딩도 필요하면 추가
        void UnbindTexture(uint32_t slot) { m_textures.erase(slot); }
        void UnbindSampler(uint32_t slot) { m_samplers.erase(slot); }

    private:
        PSOHandle m_pso;
        std::unique_ptr<UniformSet> m_uniformSet;
        std::unordered_map<uint32_t, TextureHandle> m_textures;
        std::unordered_map<uint32_t, SamplerHandle> m_samplers;
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
