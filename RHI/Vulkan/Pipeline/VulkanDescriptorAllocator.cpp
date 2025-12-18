#include "VulkanDescriptorAllocator.h"
#include "Vulkan/Logger.h"
#include <algorithm>

namespace BinRenderer::Vulkan
{
	VulkanDescriptorAllocator::VulkanDescriptorAllocator(VkDevice device)
		: device_(device)
	{
		// 기본 풀 크기 설정
		defaultPoolSizes_.uniformBuffer = 100;
		defaultPoolSizes_.storageBuffer = 50;
		defaultPoolSizes_.combinedImageSampler = 100;
		defaultPoolSizes_.storageImage = 50;
		defaultPoolSizes_.inputAttachment = 10;
		defaultPoolSizes_.maxSets = 100;
	}

	VulkanDescriptorAllocator::~VulkanDescriptorAllocator()
	{
		pools_.clear();
	}

	VkDescriptorSet VulkanDescriptorAllocator::allocate(
		VkDescriptorSetLayout layout,
		const std::vector<VkDescriptorSetLayoutBinding>& bindings)
	{
		// 1. 기존 풀에서 할당 시도
		for (auto& pool : pools_)
		{
			if (pool->canAllocate(bindings, 1))
			{
				VkDescriptorSetAllocateInfo allocInfo{};
				allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				allocInfo.descriptorPool = pool->getVkDescriptorPool();
				allocInfo.descriptorSetCount = 1;
				allocInfo.pSetLayouts = &layout;

				VkDescriptorSet descriptorSet;
				VkResult result = vkAllocateDescriptorSets(device_, &allocInfo, &descriptorSet);
				
				if (result == VK_SUCCESS)
				{
					// ✅ 용량 업데이트
					pool->updateCapacity(bindings, 1);
					return descriptorSet;
				}
			}
		}

		// 2. 새 풀 생성
		VulkanDescriptorPool* newPool = createNewPool(bindings);
		if (!newPool)
		{
			printLog("ERROR: Failed to create new descriptor pool");
			return VK_NULL_HANDLE;
		}

		// 3. 새 풀에서 할당
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = newPool->getVkDescriptorPool();
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &layout;

		VkDescriptorSet descriptorSet;
		VkResult result = vkAllocateDescriptorSets(device_, &allocInfo, &descriptorSet);
		
		if (result != VK_SUCCESS)
		{
			printLog("ERROR: Failed to allocate descriptor set from new pool");
			return VK_NULL_HANDLE;
		}

		// ✅ 용량 업데이트
		newPool->updateCapacity(bindings, 1);

		printLog("📦 Allocated descriptor set from new pool (total pools: {})", pools_.size());
		return descriptorSet;
	}

	void VulkanDescriptorAllocator::resetAll()
	{
		for (auto& pool : pools_)
		{
			pool->reset();
		}

		printLog("🔄 All descriptor pools reset");
	}

	void VulkanDescriptorAllocator::printStatistics() const
	{
		printLog("📊 Descriptor Pool Statistics:");
		printLog("  Total pools: {}", pools_.size());

		uint32_t totalSets = 0;
		uint32_t remainingSets = 0;

		for (size_t i = 0; i < pools_.size(); i++)
		{
			const auto& pool = pools_[i];
			uint32_t remaining = pool->getRemainingSetCount();
			
			printLog("  Pool {}: {} sets remaining", i, remaining);
			remainingSets += remaining;
		}

		printLog("  Total remaining sets: {}", remainingSets);
	}

	// ========================================
	// Private 메서드
	// ========================================

	VulkanDescriptorPool* VulkanDescriptorAllocator::createNewPool(
		const std::vector<VkDescriptorSetLayoutBinding>& bindings)
	{
		// Pool sizes 계산
		auto poolSizes = calculatePoolSizes(bindings);
		
		// 너무 작으면 기본 크기 사용
		if (poolSizes.empty())
		{
			poolSizes = createDefaultPoolSizes();
		}

		// 새 풀 생성
		auto pool = std::make_unique<VulkanDescriptorPool>(device_);
		if (!pool->create(defaultPoolSizes_.maxSets, poolSizes))
		{
			return nullptr;
		}

		pools_.push_back(std::move(pool));
		return pools_.back().get();
	}

	std::vector<VkDescriptorPoolSize> VulkanDescriptorAllocator::calculatePoolSizes(
		const std::vector<VkDescriptorSetLayoutBinding>& bindings) const
	{
		// 바인딩에서 필요한 타입별 개수 계산
		std::unordered_map<VkDescriptorType, uint32_t> typeCounts;

		for (const auto& binding : bindings)
		{
			typeCounts[binding.descriptorType] += binding.descriptorCount;
		}

		// 여유분 추가 (10배)
		std::vector<VkDescriptorPoolSize> poolSizes;
		for (const auto& [type, count] : typeCounts)
		{
			VkDescriptorPoolSize poolSize{};
			poolSize.type = type;
			poolSize.descriptorCount = count * 10;  // 여유분
			poolSizes.push_back(poolSize);
		}

		return poolSizes;
	}

	std::vector<VkDescriptorPoolSize> VulkanDescriptorAllocator::createDefaultPoolSizes() const
	{
		std::vector<VkDescriptorPoolSize> poolSizes;

		// Uniform Buffer
		poolSizes.push_back({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, defaultPoolSizes_.uniformBuffer });
		
		// Storage Buffer
		poolSizes.push_back({ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, defaultPoolSizes_.storageBuffer });
		
		// Combined Image Sampler
		poolSizes.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, defaultPoolSizes_.combinedImageSampler });
		
		// Storage Image
		poolSizes.push_back({ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, defaultPoolSizes_.storageImage });
		
		// Input Attachment
		poolSizes.push_back({ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, defaultPoolSizes_.inputAttachment });

		return poolSizes;
	}

} // namespace BinRenderer::Vulkan
