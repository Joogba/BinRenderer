#pragma once
#include <vector>
#include <string>
#include <cstdint>

#include "Core/Vertex.h"

namespace BinRenderer {

	struct MeshDesc {
		std::vector<Vertex> vertexBuffer; // Vertex 구조체 정의 필요
		std::vector<uint32_t> indexBuffer; // 인덱스 버퍼
		uint32_t indexCount = 0; // 인덱스 개수
	};

	struct Material {
		// TODO: 구현
	};

} // namespace BinRenderer