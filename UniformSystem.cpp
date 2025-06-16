#include "UniformSystem.h"
#include <cstring>

namespace BinRenderer {

    void UniformLayout::AddUniform(const std::string& name, uint32_t size) {
        UniformInfo info;
        info.name = name;
        info.offset = m_totalSize;
        info.size = size;
        m_uniforms.push_back(info);
        m_totalSize += size;
    }

    const UniformInfo* UniformLayout::Find(const std::string& name) const {
        for (const auto& u : m_uniforms) {
            if (u.name == name) {
                return &u;
            }
        }
        return nullptr;
    }

    UniformSet::UniformSet(std::shared_ptr<UniformLayout> layout)
        : m_layout(std::move(layout)), m_buffer(m_layout->GetTotalSize(), 0) {
    }

    void UniformSet::Set(const std::string& name, const void* data, uint32_t size) {
        const UniformInfo* info = m_layout->Find(name);
        if (info && size <= info->size) {
            std::memcpy(m_buffer.data() + info->offset, data, size);
        }
    }

} // namespace BinRenderer
