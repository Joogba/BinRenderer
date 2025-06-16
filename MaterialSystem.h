#pragma once

#include "Handle.h"
#include "PSORegistry.h"
#include "UniformSystem.h"
#include <memory>

namespace BinRenderer {



    class Material {
    public:
        Material(PSOHandle pso, std::shared_ptr<UniformLayout> layout);

        PSOHandle GetPSO() const { return m_pso; }
        UniformSet& GetUniformSet() { return *m_uniformSet; }
        const UniformSet& GetUniformSet() const { return *m_uniformSet; }

    private:
        PSOHandle m_pso;
        std::unique_ptr<UniformSet> m_uniformSet;
    };

    class MaterialRegistry {
    public:
        MaterialHandle Register(std::unique_ptr<Material> material);
        Material* Get(MaterialHandle handle);
        const Material* Get(MaterialHandle handle) const;

    private:
        std::unordered_map<uint16_t, std::unique_ptr<Material>> m_materials;
        uint16_t m_nextId = 0;
    };

} // namespace BinRenderer
