#include "PSORegistry.h"

namespace BinRenderer {

    PSOHandle PSORegistry::Register(std::unique_ptr<PipelineState> pso) {
        uint16_t id = m_nextId++;
        m_psoMap[id] = std::move(pso);
        return PSOHandle(id);
    }

    const PipelineState* PSORegistry::Get(PSOHandle handle) const {
        auto it = m_psoMap.find(handle.idx);
        if (it != m_psoMap.end()) {
            return it->second.get();
        }
        return nullptr;
    }

} // namespace BinRenderer