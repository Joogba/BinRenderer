#pragma once

#include <glm/glm.hpp>
#include <cstdint>

namespace BinRenderer
{
	/**
	 * @brief GPU Instancing용 Per-instance 데이터
	 * 
	 * 16-byte aligned for GPU buffer
	 * Binding 1에서 VK_VERTEX_INPUT_RATE_INSTANCE로 사용
	 */
	struct InstanceData
	{
		glm::mat4 modelMatrix;       // 64 bytes - Per-instance transform
		uint32_t materialOffset;     // 4 bytes  - Per-instance material override
		uint32_t padding[3];         // 12 bytes - Padding for 16-byte alignment

		InstanceData()
			: modelMatrix(1.0f)
			, materialOffset(0)
			, padding{ 0, 0, 0 }
		{
		}

		InstanceData(const glm::mat4& model, uint32_t matOffset = 0)
			: modelMatrix(model)
			, materialOffset(matOffset)
			, padding{ 0, 0, 0 }
		{
		}
	};

	static_assert(sizeof(InstanceData) == 80, "InstanceData must be 80 bytes (64 + 4 + 12)");
	static_assert(sizeof(InstanceData) % 16 == 0, "InstanceData must be 16-byte aligned");

} // namespace BinRenderer
