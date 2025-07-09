#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include "Core/Handle.h"

namespace BinRenderer {

	struct Viewport {
		float x = 0.0f;
		float y = 0.0f;
		float width = 0.0f;
		float height = 0.0f;
		float minDepth = 0.0f;
		float maxDepth = 1.0f;
	};

	struct View
	{
		// 렌더 타겟 + 깊이뷰
		std::vector<RenderTargetViewHandle> rtvs;   // 멀티타겟 지원
		DepthStencilViewHandle dsv;

		// 클리어 설정
		uint32_t clearFlags = 0;          // enum class ClearFlags 비트마스크
		uint32_t clearColor = 0xff000000; // AARRGGBB (or float[4])
		float    clearDepth = 1.0f;
		uint8_t  clearStencil = 0;

		// 뷰포트
		Viewport viewport;

		std::string name;
		// glm::mat4 viewMatrix;
	   // glm::mat4 projMatrix;
	};

} // namespace BinRenderer
