// PSORegistry.h
#pragma once
#include <unordered_map>
#include <string>
#include <functional>

#include "Core/Handle.h"
#include "Core/RenderEnums.h"
#include "Core/RenderStates.h"

namespace BinRenderer {

    // 해시 결합 유틸리티
    inline void HashCombine(size_t& seed, size_t value) {
        seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    inline bool operator==(const PSODesc& a, const PSODesc& b) {
        return a.vertexShader == b.vertexShader &&
            a.pixelShader == b.pixelShader &&
            a.hullShader == b.hullShader &&
            a.domainShader == b.domainShader &&
            a.geometryShader == b.geometryShader &&
            a.inputLayout == b.inputLayout &&
            a.blendState == b.blendState &&
            a.depthStencilState == b.depthStencilState &&
            a.rasterizerState == b.rasterizerState &&
            a.blendFactor == b.blendFactor &&
            a.stencilRef == b.stencilRef &&
            a.primitiveTopology == b.primitiveTopology &&
            a.sampleMask == b.sampleMask &&
            a.rtvFormats == b.rtvFormats &&
            a.dsvFormat == b.dsvFormat &&
            a.numRenderTargets == b.numRenderTargets;
    }

    



    class PSORegistry {
    public:
        PSOHandle Register(const std::string& name, const PSODesc& pso) {
            auto it = m_nameToIdx.find(name);
            if (it != m_nameToIdx.end()) return PSOHandle(it->second);
            PSOHandle handle(m_nextId++);
            m_psos.emplace(handle.idx, pso);
            m_nameToIdx[name] = handle.idx;
            m_idxToName[handle.idx] = name;
            return handle;
        }

        const PSODesc* Get(PSOHandle handle) const {
            auto it = m_psos.find(handle.idx);
            return it != m_psos.end() ? &it->second : nullptr;
        }

        const PSODesc* Get(const std::string& name) const {
            auto it = m_nameToIdx.find(name);
            return it != m_nameToIdx.end() ? Get(PSOHandle(it->second)) : nullptr;
        }

        PSOHandle GetHandle(const std::string& name) const {
            auto it = m_nameToIdx.find(name);
            return it != m_nameToIdx.end() ? PSOHandle(it->second) : PSOHandle();
        }

        const std::string& GetName(PSOHandle handle) const {
            static std::string empty;
            auto it = m_idxToName.find(handle.idx);
            return it != m_idxToName.end() ? it->second : empty;
        }

    private:
        std::unordered_map<uint16_t, PSODesc> m_psos;
        std::unordered_map<std::string, uint16_t> m_nameToIdx;
        std::unordered_map<uint16_t, std::string> m_idxToName;
        uint16_t m_nextId = 1;
    };

} // namespace BinRenderer
