#pragma once
#include <unordered_map>
#include <string>
#include "Core/Handle.h"
#include "Core/RenderStates.h"

namespace BinRenderer {

    class ShaderRegistry {
    public:
        ShaderHandle Register(const std::string& name, const ShaderDesc& shader) {
            auto it = m_nameToId.find(name);
            if (it != m_nameToId.end()) return ShaderHandle(it->second);
            ShaderHandle handle(m_nextId++);
            m_shaders.emplace(handle.idx, shader);
            m_nameToId[name] = handle.idx;
            m_idToName[handle.idx] = name;
            return handle;
        }

        const ShaderDesc* Get(ShaderHandle handle) const {
            auto it = m_shaders.find(handle.idx);
            return it != m_shaders.end() ? &it->second : nullptr;
        }

        const ShaderDesc* Get(const std::string& name) const {
            auto it = m_nameToId.find(name);
            return it != m_nameToId.end() ? Get(ShaderHandle(it->second)) : nullptr;
        }

        ShaderHandle GetHandle(const std::string& name) const {
            auto it = m_nameToId.find(name);
            return it != m_nameToId.end() ? ShaderHandle(it->second) : ShaderHandle();
        }

        const std::string& GetName(ShaderHandle handle) const {
            static std::string empty;
            auto it = m_idToName.find(handle.idx);
            return it != m_idToName.end() ? it->second : empty;
        }

    private:
        std::unordered_map<uint32_t, ShaderDesc> m_shaders;
        std::unordered_map<std::string, uint32_t> m_nameToId;
        std::unordered_map<uint32_t, std::string> m_idToName;
        uint32_t m_nextId = 1;
    };

}
