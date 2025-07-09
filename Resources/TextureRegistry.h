#pragma once
#include "Core/Handle.h"

#include <unordered_map>
#include <string>

namespace BinRenderer {

    struct Texture { /* ... */ };

    class TextureRegistry {
    public:
        TextureHandle Register(const std::string& name, const Texture& tex) {
            auto it = m_nameToIdx.find(name);
            if (it != m_nameToIdx.end()) return TextureHandle(it->second);
            TextureHandle handle(m_nextId++);
            m_textures.emplace(handle.idx, tex);
            m_nameToIdx[name] = handle.idx;
            m_idxToName[handle.idx] = name;
            return handle;
        }

        const Texture* Get(TextureHandle handle) const {
            auto it = m_textures.find(handle.idx);
            return it != m_textures.end() ? &it->second : nullptr;
        }

        const Texture* Get(const std::string& name) const {
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
        std::unordered_map<uint32_t, Texture> m_textures;
        std::unordered_map<std::string, uint32_t> m_nameToIdx;
        std::unordered_map<uint32_t, std::string> m_idxToName;
        uint32_t m_nextId = 1;
    };
}