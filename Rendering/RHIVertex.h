#pragma once

#include <glm/glm.hpp>
#include <cstdint>

namespace BinRenderer
{
	/**
	 * @brief 플랫폼 독립적 정점 구조
	 */
	struct RHIVertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 texCoord;
		glm::vec4 tangent;  // w = handedness

		// Skeletal animation (optional)
		glm::vec4 boneWeights = glm::vec4(0.0f);
		glm::ivec4 boneIndices = glm::ivec4(-1);

		RHIVertex() = default;

		RHIVertex(const glm::vec3& pos, const glm::vec3& norm, const glm::vec2& uv)
			: position(pos), normal(norm), texCoord(uv), tangent(0.0f)
		{
		}
	};

	/**
	 * @brief 정점 형식 설명
	 */
	enum class RHIVertexFormat
	{
		Position,        // vec3
		PositionNormal,     // vec3 + vec3
		PositionNormalUV,   // vec3 + vec3 + vec2
		PositionNormalUVTangent, // vec3 + vec3 + vec2 + vec4
		PositionNormalUVTangentSkinned, // + vec4 weights + ivec4 indices
	};

} // namespace BinRenderer
