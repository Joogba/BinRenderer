#include "UniformSystem.h"
#include <cstring>
#include <cassert>

namespace BinRenderer {

	void UniformLayout::AddUniform(const std::string& name, uint32_t size) {
		UniformInfo info;
		info.name = name;
		info.offset = m_totalSize;
		info.size = size;
		m_uniforms.push_back(info);
		m_totalSize += size;
	}

	void UniformLayout::AddPredefined(PredefinedUniformType type, uint32_t size) {
		PredefinedUniformInfo pi;
		pi.type = type;
		pi.offset = m_totalSize;
		pi.size = size;
		m_predefineds.push_back(pi);
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

	void UniformSet::ApplyPredefined(PredefinedUniformType type, const void* data, uint32_t size) {
		// layout�� ����� predefined ��� �߿��� ��ġ�ϴ� Ÿ�� ã��
		for (const auto& pi : m_layout->GetPredefineds()) {
			if (pi.type == type) {
				assert(size <= pi.size);
				std::memcpy(m_buffer.data() + pi.offset, data, size);
				return;

			}

		}

	}

} // namespace BinRenderer
