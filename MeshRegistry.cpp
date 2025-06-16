#include "MeshRegistry.h"

namespace BinRenderer {

    MeshHandle MeshRegistry::Register(const Mesh& mesh) {
        uint16_t id = m_nextId++;
        m_meshes[id] = mesh;
        return MeshHandle(id);
    }

    const Mesh* MeshRegistry::Get(MeshHandle handle) const {
        auto it = m_meshes.find(handle.idx);
        if (it != m_meshes.end()) {
            return &it->second;
        }
        return nullptr;
    }

} // namespace BinRenderer