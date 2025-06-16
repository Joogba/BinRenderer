
#include "MaterialSystem.h"

namespace BinRenderer {

    Material::Material(PSOHandle pso, std::shared_ptr<UniformLayout> layout)
        : m_pso(pso), m_uniformSet(std::make_unique<UniformSet>(layout)) {
    }

    MaterialHandle MaterialRegistry::Register(std::unique_ptr<Material> material) {
        uint16_t id = m_nextId++;
        m_materials[id] = std::move(material);
        return MaterialHandle(id);
    }

    // 상수 버전 (진짜 구현 위치)
    const Material* MaterialRegistry::Get(MaterialHandle handle) const {
        auto it = m_materials.find(handle.idx);
        return (it != m_materials.end()) ? it->second.get() : nullptr;
    }

    // 비상수 버전 → 상수 버전 호출
    Material* MaterialRegistry::Get(MaterialHandle handle) {
        return const_cast<Material*>(static_cast<const MaterialRegistry*>(this)->Get(handle));
    }

} // namespace BinRenderer
