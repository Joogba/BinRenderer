
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

    // ��� ���� (��¥ ���� ��ġ)
    const Material* MaterialRegistry::Get(MaterialHandle handle) const {
        auto it = m_materials.find(handle.idx);
        return (it != m_materials.end()) ? it->second.get() : nullptr;
    }

    // ���� ���� �� ��� ���� ȣ��
    Material* MaterialRegistry::Get(MaterialHandle handle) {
        return const_cast<Material*>(static_cast<const MaterialRegistry*>(this)->Get(handle));
    }

} // namespace BinRenderer
