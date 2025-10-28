#pragma once

namespace Scene
{
	enum class ELightType
	{
		AMBIENT = 0,
		DIRECTIONAL,
		POINT,
		SPOT,
		PATH_TRACING,		// Specific type of light is defined in jPathTracingLight
		MAX
	};


}
