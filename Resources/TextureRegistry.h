#pragma once
#include "Core/Handle.h"
#include "Core/RenderEnums.h"

#include <unordered_map>
#include <string>
#include <cstdint>



namespace BinRenderer {

    class TextureRegistry {
    public:
        TextureHandle Register(const std::string& name, const TextureDesc& tex) {
            auto it = m_nameToIdx.find(name);
            if (it != m_nameToIdx.end()) return TextureHandle(it->second);
            TextureHandle handle(m_nextId++);
            m_textures.emplace(handle.idx, tex);
            m_nameToIdx[name] = handle.idx;
            m_idxToName[handle.idx] = name;
            return handle;
        }

        const TextureDesc* Get(TextureHandle handle) const {
            auto it = m_textures.find(handle.idx);
            return it != m_textures.end() ? &it->second : nullptr;
        }

        const TextureDesc* Get(const std::string& name) const {
            auto it = m_nameToIdx.find(name);
            return it != m_nameToIdx.end() ? Get(TextureHandle(it->second)) : nullptr;
        }

        TextureHandle GetHandle(const std::string& name) const {
            auto it = m_nameToIdx.find(name);
            return it != m_nameToIdx.end() ? TextureHandle(it->second) : TextureHandle();
        }

        const std::string& GetName(TextureHandle handle) const {
            static std::string empty;
            auto it = m_idxToName.find(handle.idx);
            return it != m_idxToName.end() ? it->second : empty;
        }

    private:
        std::unordered_map<uint32_t, TextureDesc> m_textures;
        std::unordered_map<std::string, uint32_t> m_nameToIdx;
        std::unordered_map<uint32_t, std::string> m_idxToName;
        uint32_t m_nextId = 1;
    };

}

namespace std {
    template<>
    struct std::hash <BinRenderer::TextureDesc> {
        size_t operator()(const BinRenderer::TextureDesc& d) const {
            return hash<uint32_t>()(d.width) ^ hash<uint32_t>()(d.height) ^ hash<uint32_t>()((uint32_t)d.format);
        }
    };
}