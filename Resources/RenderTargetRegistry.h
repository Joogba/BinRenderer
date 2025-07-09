#pragma once

#include <unordered_map>
#include <string>
#include "Core/Handle.h"

namespace BinRenderer {

    struct RenderTargetDesc { /* ... */ };

    class RenderTargetRegistry {
    public:
        RenderTargetViewHandle Register(const std::string& name, const RenderTargetDesc& rtv) {
            auto it = m_nameToIdx.find(name);
            if (it != m_nameToIdx.end()) return RenderTargetViewHandle(it->second);
            RenderTargetViewHandle handle(m_nextId++);
            m_renderTargets.emplace(handle.idx, rtv);
            m_nameToIdx[name] = handle.idx;
            m_idxToName[handle.idx] = name;
            return handle;
        }

        const RenderTargetDesc* Get(RenderTargetViewHandle handle) const {
            auto it = m_renderTargets.find(handle.idx);
            return it != m_renderTargets.end() ? &it->second : nullptr;
        }

        const RenderTargetDesc* Get(const std::string& name) const {
            auto it = m_nameToIdx.find(name);
            return it != m_nameToIdx.end() ? Get(RenderTargetViewHandle(it->second)) : nullptr;
        }

        RenderTargetViewHandle GetHandle(const std::string& name) const {
            auto it = m_nameToIdx.find(name);
            return it != m_nameToIdx.end() ? RenderTargetViewHandle(it->second) : RenderTargetViewHandle();
        }

        const std::string& GetName(RenderTargetViewHandle handle) const {
            static std::string empty;
            auto it = m_idxToName.find(handle.idx);
            return it != m_idxToName.end() ? it->second : empty;
        }

    private:
        std::unordered_map<uint16_t, RenderTargetDesc> m_renderTargets;
        std::unordered_map<std::string, uint16_t> m_nameToIdx;
        std::unordered_map<uint16_t, std::string> m_idxToName;
        uint16_t m_nextId = 1;
    };

}