#pragma once

#include "../Core/RHIType.h"
#include "../Core/RHIHandle.h"
#include "RHIImageStructs.h"
#include "RHIBufferStructs.h"

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
		const RHISampler* const* pImmutableSamplers = nullptr;
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

	struct RHIDescriptorSetAllocateInfo
    {
        RHIDescriptorPool* descriptorPool;
        uint32_t descriptorSetCount;
        const RHIDescriptorSetLayout* pSetLayouts;
    };

	struct RHICopyDescriptorSet
    {
        RHIDescriptorSet* srcSet;
        uint32_t srcBinding;
        uint32_t srcArrayElement;
        RHIDescriptorSet* dstSet;
        uint32_t dstBinding;
        uint32_t dstArrayElement;
        uint32_t descriptorCount;
    };

	struct RHIDescriptorImageInfo
    {
        RHISampler* sampler;
        RHIImageView* imageView;
        RHIImageLayout imageLayout;
    };

	struct RHIDescriptorBufferInfo
    {
        RHIBuffer* buffer;
        RHIDeviceSize offset;
        RHIDeviceSize range;
    };

	struct RHIWriteDescriptorSet
    {
        RHIDescriptorSet* dstSet;
        uint32_t dstBinding;
        uint32_t dstArrayElement;
        uint32_t descriptorCount;
        RHIDescriptorType descriptorType;
        const RHIDescriptorImageInfo* pImageInfo;
        const RHIDescriptorBufferInfo* pBufferInfo;
        const RHIBufferView* pTexelBufferView;
    };

} // namespace BinRenderer
