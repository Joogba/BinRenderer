#pragma once
#include <unordered_map>
#include <string>
#include "Core/Handle.h"
#include "Core/RenderStates.h"

namespace BinRenderer {

    class SamplerRegistry {
    public:
        SamplerHandle Register(const std::string& name, const SamplerDesc& desc) {
            auto it = m_nameToIdx.find(name);
            if (it != m_nameToIdx.end()) return SamplerHandle(it->second);
            SamplerHandle handle(m_nextId++);
            m_samplers.insert({handle.idx, desc});
            m_nameToIdx[name] = handle.idx;
            m_idxToName[handle.idx] = name;
            return handle;
        }

        const SamplerDesc* Get(SamplerHandle handle) const {
            auto it = m_samplers.find(handle.idx);
            return it != m_samplers.end() ? &it->second : nullptr;
        }

        const SamplerDesc* Get(const std::string& name) const {
            auto it = m_nameToIdx.find(name);
            return it != m_nameToIdx.end() ? Get(SamplerHandle(it->second)) : nullptr;
        }

        SamplerHandle GetHandle(const std::string& name) const {
            auto it = m_nameToIdx.find(name);
            return it != m_nameToIdx.end() ? SamplerHandle(it->second) : SamplerHandle();
        }

        const std::string& GetName(SamplerHandle handle) const {
            static std::string empty;
            auto it = m_idxToName.find(handle.idx);
            return it != m_idxToName.end() ? it->second : empty;
        }

    private:
        std::unordered_map<uint32_t, SamplerDesc> m_samplers;
        std::unordered_map<std::string, uint32_t> m_nameToIdx;
        std::unordered_map<uint32_t, std::string> m_idxToName;
        uint32_t m_nextId = 1;
    };

}