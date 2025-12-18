#pragma once

#include <glm/glm.hpp>
#include <string>
#include <cstdint>

namespace BinRenderer
{
	/**
	 * @brief 플랫폼 독립적 Material 데이터
	 */
	struct MaterialData
	{
		glm::vec4 emissiveFactor = glm::vec4(0.0f);
		glm::vec4 baseColorFactor = glm::vec4(1.0f);
		float roughness = 1.0f;
		float metallic = 0.0f;
		float transparency = 1.0f;
		float discardAlpha = 0.0f;

		// Texture indices (bindless)
		int32_t baseColorTextureIndex = -1;
		int32_t normalTextureIndex = -1;
		int32_t metallicRoughnessTextureIndex = -1;
		int32_t emissiveTextureIndex = -1;
		int32_t occlusionTextureIndex = -1;
		int32_t opacityTextureIndex = -1;

		// Flags
		enum Flags : uint32_t
		{
			CastShadow = 0x1,
			ReceiveShadow = 0x2,
			Transparent = 0x4,
		};
		uint32_t flags = CastShadow | ReceiveShadow;

		std::string name;
	};

	/**
	 * @brief RHI 기반 Material 클래스
	 */
	class RHIMaterial
	{
	public:
		RHIMaterial() = default;
		~RHIMaterial() = default;

		// Material 데이터 접근
		MaterialData& getData() { return data_; }
		const MaterialData& getData() const { return data_; }

		// 캐시 저장/로드
		void loadFromCache(const std::string& cachePath);
		void writeToCache(const std::string& cachePath);

	private:
		MaterialData data_;
	};

} // namespace BinRenderer
