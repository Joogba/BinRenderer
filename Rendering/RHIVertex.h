#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/packing.hpp>  // For half-precision support
#include <cstdint>
#include <vector>
#include "../RHI/Structs/RHIPipelineCreateInfo.h"

namespace BinRenderer
{
	// Half-precision type aliases for clarity and consistency
	using half = uint16_t;  // Using uint16_t to represent packed half-float values

	// Custom packed half-precision vector types for optimal memory layout
	struct alignas(2) hvec2 {
		half x, y;
		hvec2() : x(0), y(0) {}
		hvec2(half x_, half y_) : x(x_), y(y_) {}
	};

	struct alignas(2) hvec3 {
		half x, y, z;
		hvec3() : x(0), y(0), z(0) {}
		hvec3(half x_, half y_, half z_) : x(x_), y(y_), z(z_) {}
	};

	struct alignas(2) hvec4 {
		half x, y, z, w;
		hvec4() : x(0), y(0), z(0), w(0) {}
		hvec4(half x_, half y_, half z_, half w_) : x(x_), y(y_), z(z_), w(w_) {}
	};

	// Helper functions for half-precision conversion
	inline half packHalf(float value) {
		return glm::packHalf1x16(value);
	}

	inline float unpackHalf(half value) {
		return glm::unpackHalf1x16(value);
	}

	inline hvec2 packHalf2(const glm::vec2& value) {
		return hvec2(packHalf(value.x), packHalf(value.y));
	}

	inline glm::vec2 unpackHalf2(const hvec2& value) {
		return glm::vec2(unpackHalf(value.x), unpackHalf(value.y));
	}

	inline hvec3 packHalf3(const glm::vec3& value) {
		return hvec3(packHalf(value.x), packHalf(value.y), packHalf(value.z));
	}

	inline glm::vec3 unpackHalf3(const hvec3& value) {
		return glm::vec3(unpackHalf(value.x), unpackHalf(value.y), unpackHalf(value.z));
	}

	inline hvec4 packHalf4(const glm::vec4& value) {
		return hvec4(packHalf(value.x), packHalf(value.y), packHalf(value.z), packHalf(value.w));
	}

	inline glm::vec4 unpackHalf4(const hvec4& value) {
		return glm::vec4(unpackHalf(value.x), unpackHalf(value.y), unpackHalf(value.z), unpackHalf(value.w));
	}

	/**
	 * @brief 플랫폼 독립적 정점 구조
	 * 
	 * Half-precision을 사용하여 메모리 대역폭 최적화:
	 * - position: hvec3 (6 bytes) - half-precision position
	 * - normal: hvec3 (6 bytes) - half-precision normal
	 * - texCoord: hvec2 (4 bytes) - half-precision texture coordinates
	 * - tangent: hvec3 (6 bytes) - half-precision tangent
	 * - bitangent: hvec3 (6 bytes) - half-precision bitangent
	 * - boneWeights: vec4 (16 bytes) - full precision for animation accuracy
	 * - boneIndices: ivec4 (16 bytes) - full precision bone indices
	 * Total size: ~60 bytes (레거시 88 bytes에서 32% 감소)
	 */
	struct RHIVertex
	{
		hvec3 position;     // 6 bytes - half-precision position
		hvec3 normal;       // 6 bytes - half-precision normal
		hvec2 texCoord;     // 4 bytes - half-precision texture coordinates
		hvec3 tangent;      // 6 bytes - half-precision tangent
		hvec3 bitangent;    // 6 bytes - half-precision bitangent

		// Skeletal animation (full precision for accuracy)
		alignas(4) glm::vec4 boneWeights = glm::vec4(0.0f);
		alignas(4) glm::ivec4 boneIndices = glm::ivec4(-1);

		RHIVertex() 
			: position(packHalf3(glm::vec3(0.0f)))
			, normal(packHalf3(glm::vec3(0.0f, 1.0f, 0.0f)))
			, texCoord(packHalf2(glm::vec2(0.0f)))
			, tangent(packHalf3(glm::vec3(1.0f, 0.0f, 0.0f)))
			, bitangent(packHalf3(glm::vec3(0.0f, 0.0f, 1.0f)))
		{
		}

		RHIVertex(const glm::vec3& pos, const glm::vec3& norm, const glm::vec2& uv)
			: position(packHalf3(pos))
			, normal(packHalf3(norm))
			, texCoord(packHalf2(uv))
			, tangent(packHalf3(glm::vec3(1.0f, 0.0f, 0.0f)))
			, bitangent(packHalf3(glm::vec3(0.0f, 0.0f, 1.0f)))
		{
		}

		// Accessor methods for unpacked values
		glm::vec3 getPosition() const { return unpackHalf3(position); }
		glm::vec3 getNormal() const { return unpackHalf3(normal); }
		glm::vec2 getTexCoord() const { return unpackHalf2(texCoord); }
		glm::vec3 getTangent() const { return unpackHalf3(tangent); }
		glm::vec3 getBitangent() const { return unpackHalf3(bitangent); }

		// Setter methods with automatic packing
		void setPosition(const glm::vec3& pos) { position = packHalf3(pos); }
		void setNormal(const glm::vec3& norm) { normal = packHalf3(norm); }
		void setTexCoord(const glm::vec2& tex) { texCoord = packHalf2(tex); }
		void setTangent(const glm::vec3& tan) { tangent = packHalf3(tan); }
		void setBitangent(const glm::vec3& bitan) { bitangent = packHalf3(bitan); }
	};

	/**
	 * @brief 정점 형식 설명
	 */
	enum class RHIVertexFormat
	{
		Position,        // hvec3
		PositionNormal,     // hvec3 + hvec3
		PositionNormalUV,   // hvec3 + hvec3 + hvec2
		PositionNormalUVTangent, // hvec3 + hvec3 + hvec2 + hvec3 + hvec3
		PositionNormalUVTangentSkinned, // + vec4 weights + ivec4 indices
	};

	/**
	 * @brief RHIVertex용 Vertex Input 헬퍼
	 */
	namespace RHIVertexHelper
	{
		/**
		 * @brief Vertex buffer의 Binding Description 반환
		 * @return Binding 0에 대한 설정
		 */
		inline RHIVertexInputBinding getVertexBinding()
		{
			RHIVertexInputBinding binding;
			binding.binding = 0;
			binding.stride = sizeof(RHIVertex);  // ~60 bytes
			binding.inputRate = RHI_VERTEX_INPUT_RATE_VERTEX;
			return binding;
		}

		/**
		 * @brief Basic vertex attributes (location 0-4, without bone data) 반환
		 */
		inline std::vector<RHIVertexInputAttribute> getVertexAttributesBasic()
		{
			std::vector<RHIVertexInputAttribute> attributes(5);

			// Position (location 0) - hvec3
			attributes[0].location = 0;
			attributes[0].binding = 0;
			attributes[0].format = RHI_FORMAT_R16G16B16_SFLOAT;
			attributes[0].offset = offsetof(RHIVertex, position);

			// Normal (location 1) - hvec3
			attributes[1].location = 1;
			attributes[1].binding = 0;
			attributes[1].format = RHI_FORMAT_R16G16B16_SFLOAT;
			attributes[1].offset = offsetof(RHIVertex, normal);

			// TexCoord (location 2) - hvec2
			attributes[2].location = 2;
			attributes[2].binding = 0;
			attributes[2].format = RHI_FORMAT_R16G16_SFLOAT;
			attributes[2].offset = offsetof(RHIVertex, texCoord);

			// Tangent (location 3) - hvec3
			attributes[3].location = 3;
			attributes[3].binding = 0;
			attributes[3].format = RHI_FORMAT_R16G16B16_SFLOAT;
			attributes[3].offset = offsetof(RHIVertex, tangent);

			// Bitangent (location 4) - hvec3
			attributes[4].location = 4;
			attributes[4].binding = 0;
			attributes[4].format = RHI_FORMAT_R16G16B16_SFLOAT;
			attributes[4].offset = offsetof(RHIVertex, bitangent);

			return attributes;
		}

		/**
		 * @brief Animated vertex attributes (location 0-6, with bone data) 반환
		 */
		inline std::vector<RHIVertexInputAttribute> getVertexAttributesAnimated()
		{
			auto attributes = getVertexAttributesBasic();
			attributes.resize(7);

			// Bone weights (location 5) - vec4
			attributes[5].location = 5;
			attributes[5].binding = 0;
			attributes[5].format = RHI_FORMAT_R32G32B32A32_SFLOAT;
			attributes[5].offset = offsetof(RHIVertex, boneWeights);

			// Bone indices (location 6) - ivec4
			attributes[6].location = 6;
			attributes[6].binding = 0;
			attributes[6].format = RHI_FORMAT_R32G32B32A32_SINT;
			attributes[6].offset = offsetof(RHIVertex, boneIndices);

			return attributes;
		}
	}

	// Compile-time validation of vertex structure layout
	static_assert(sizeof(glm::vec4) == 16, "vec4 must be 16 bytes");
	static_assert(sizeof(glm::ivec4) == 16, "ivec4 must be 16 bytes");
	static_assert(sizeof(half) == 2, "half must be 2 bytes");
	static_assert(sizeof(hvec2) == 4, "hvec2 must be 4 bytes");
	static_assert(sizeof(hvec3) == 6, "hvec3 must be 6 bytes");
	static_assert(sizeof(hvec4) == 8, "hvec4 must be 8 bytes");
	static_assert(sizeof(RHIVertex) <= 64,
		"RHIVertex size should be around 60 bytes for significant memory savings");

} // namespace BinRenderer
