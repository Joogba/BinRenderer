#pragma once

#include "../Core/RHIType.h"
#include <vector>
#include <string>

namespace BinRenderer
{
	/**
	 * @brief 셰이더 모듈 추상 클래스
  */
	class RHIShader
	{
	public:
		virtual ~RHIShader() = default;

		virtual RHIShaderStageFlags getStage() const = 0;
		virtual const std::string& getName() const = 0;
		virtual const std::string& getEntryPoint() const = 0;
	};

} // namespace BinRenderer
