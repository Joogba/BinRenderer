#pragma once

#include "../Core/RHIType.h"
#include <vector>
#include <string>

namespace BinRenderer
{
	/**
	 * @brief 셰이더 생성 정보
   */
	struct RHIShaderCreateInfo
	{
		RHIShaderStageFlags stage = 0;
		const char* entryPoint = "main";
		std::vector<uint32_t> code;
		std::string name;
	};

} // namespace BinRenderer
