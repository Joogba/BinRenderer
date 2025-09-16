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

} // namespace BinRenderer
