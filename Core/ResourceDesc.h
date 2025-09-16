#pragma once

namespace BinRenderer {
	struct TextureDesc {
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t mipLevels = 1;
		uint32_t arraySize = 1;
		uint32_t sampleCount = 1;
		bool isCubeMap = false;
		bool isRenderTarget = false;
		bool isDepthStencil = false;
		uint32_t flags = 0; // 예: TEXTURE_FLAG_GENERATE_MIPS
	};

	struct SameplerDesc {
		uint32_t filter = 0; // 예: FILTER_LINEAR
		uint32_t addressU = 0; // 예: TEXTURE_ADDRESS_WRAP
		uint32_t addressV = 0; // 예: TEXTURE_ADDRESS_WRAP
		uint32_t addressW = 0; // 예: TEXTURE_ADDRESS_WRAP
		float mipLODBias = 0.0f;
		uint32_t maxAnisotropy = 1;
		float borderColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		float minLOD = 0.0f;
		float maxLOD = FLT_MAX;
		uint32_t comparisonFunc = 0; // 예: COMPARISON_NEVER
	};

	struct TextureDesc {
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t mipLevels = 1;
		uint32_t arraySize = 1;
		uint32_t sampleCount = 1;
		bool isCubeMap = false;
		bool isRenderTarget = false;
		bool isDepthStencil = false;
		uint32_t flags = 0; // 예: TEXTURE_FLAG_GENERATE_MIPS
	};

	struct MeshDesc {
		std::vector<Vertex> vertexBuffer; // Vertex 구조체 정의 필요
		std::vector<uint32_t> indexBuffer; // 인덱스 버퍼
		uint32_t indexCount = 0; // 인덱스 개수
	};

	struct Material {

	}

	struct PSODesc {
		std::string name;
		uint32_t sampleMask = 0xFFFFFFFF; // 샘플 마스크
		uint32_t primitiveTopology = 0; // 예: PRIMITIVE_TOPOLOGY_TRIANGLELIST
		BlendState blendState; // 블렌드 상태
		DepthStencilState depthStencilState; // 깊이·스텐실 상태
		RasterizerState rasterizerState; // 래스터라이저 상태
	};

} // namespace BinRenderer