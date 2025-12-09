#pragma once

#include "../Core/RHIType.h"
#include "../Pipeline/RHIDescriptor.h"

namespace BinRenderer
{
	/**
 * @brief 디스크립터 셋 레이아웃 바인딩
   */
	struct RHIDescriptorSetLayoutBinding
	{
		uint32_t binding = 0;
		RHIDescriptorType descriptorType = RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uint32_t descriptorCount = 1;
		RHIShaderStageFlags stageFlags = 0;
	};

	/**
	 * @brief 디스크립터 셋 레이아웃 생성 정보
 */
	struct RHIDescriptorSetLayoutCreateInfo
	{
		std::vector<RHIDescriptorSetLayoutBinding> bindings;
	};

	/**
		* @brief 디스크립터 풀 크기
	 */
	struct RHIDescriptorPoolSize
	{
		RHIDescriptorType type = RHI_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uint32_t descriptorCount = 0;
	};

	/**
	   * @brief 디스크립터 풀 생성 정보
	 */
	struct RHIDescriptorPoolCreateInfo
	{
		uint32_t maxSets = 0;
		std::vector<RHIDescriptorPoolSize> poolSizes;
	};

} // namespace BinRenderer
