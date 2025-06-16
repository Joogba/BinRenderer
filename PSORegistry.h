// PSORegistry.h
#pragma once

#include "PipelineState.h"
#include "Handle.h"
#include <unordered_map>
#include <memory>
#include <cstdint>

namespace BinRenderer {

    

    class PSORegistry {
    public:
        PSOHandle Register(std::unique_ptr<PipelineState> pso);
        const PipelineState* Get(PSOHandle handle) const;

    private:
        std::unordered_map<uint16_t, std::unique_ptr<PipelineState>> m_psoMap;
        uint16_t m_nextId = 0;
    };

} // namespace BinRenderer
