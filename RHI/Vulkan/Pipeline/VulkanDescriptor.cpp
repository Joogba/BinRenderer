#include "VulkanDescriptor.h"
#include "../Resources/VulkanBuffer.h"
#include "../Resources/VulkanImage.h"
#include "../Resources/VulkanSampler.h"
#include "Vulkan/Logger.h"

namespace BinRenderer::Vulkan
{
	// ========================================
	// VulkanDescriptorSetLayout
 // ========================================

	VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VkDevice device)
		: device_(device)
	{
	}

	VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
	{
		destroy();
	}

	bool VulkanDescriptorSetLayout::create(const std::vector<VkDescriptorSetLayoutBinding>& bindings)
	{
		bindingCount_ = static_cast<uint32_t>(bindings.size());
		bindings_ = bindings;  // ✅ Binding 정보 저장

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(device_, &layoutInfo, nullptr, &layout_) != VK_SUCCESS)
		{
			return false;
		}

		return true;
	}

	void VulkanDescriptorSetLayout::destroy()
	{
		if (layout_ != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorSetLayout(device_, layout_, nullptr);
			layout_ = VK_NULL_HANDLE;
		}
		bindings_.clear();  // ✅ Binding 정보 초기화
	}

	// ========================================
 // VulkanDescriptorPool
	// ========================================

	VulkanDescriptorPool::VulkanDescriptorPool(VkDevice device)
		: device_(device)
	{
	}

	VulkanDescriptorPool::~VulkanDescriptorPool()
	{
		destroy();
	}

	bool VulkanDescriptorPool::create(uint32_t maxSets, const std::vector<VkDescriptorPoolSize>& poolSizes)
	{
		maxSets_ = maxSets;
		remainingSets_ = maxSets;

		// ✅ 용량 추적 초기화
		for (const auto& poolSize : poolSizes)
		{
			totalDescriptors_[poolSize.type] = poolSize.descriptorCount;
			remainingDescriptors_[poolSize.type] = poolSize.descriptorCount;
		}

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = maxSets;

		if (vkCreateDescriptorPool(device_, &poolInfo, nullptr, &pool_) != VK_SUCCESS)
		{
			return false;
		}

		printLog("✅ Descriptor pool created: {} sets, {} types", maxSets, poolSizes.size());
		return true;
	}

	void VulkanDescriptorPool::destroy()
	{
		if (pool_ != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorPool(device_, pool_, nullptr);
			pool_ = VK_NULL_HANDLE;
		}

		// ✅ 용량 추적 초기화
		maxSets_ = 0;
		remainingSets_ = 0;
		totalDescriptors_.clear();
		remainingDescriptors_.clear();
	}

	void VulkanDescriptorPool::reset()
	{
		if (pool_ != VK_NULL_HANDLE)
		{
			vkResetDescriptorPool(device_, pool_, 0);

			// ✅ 용량 리셋
			remainingSets_ = maxSets_;
			remainingDescriptors_ = totalDescriptors_;
		}
	}

	RHIDescriptorSet* VulkanDescriptorPool::allocateDescriptorSet(RHIDescriptorSetLayout* layout)
	{
		auto* vulkanLayout = static_cast<VulkanDescriptorSetLayout*>(layout);

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = pool_;
		allocInfo.descriptorSetCount = 1;
		VkDescriptorSetLayout vkLayout = vulkanLayout->getVkDescriptorSetLayout();
		allocInfo.pSetLayouts = &vkLayout;

		VkDescriptorSet descriptorSet;
		if (vkAllocateDescriptorSets(device_, &allocInfo, &descriptorSet) != VK_SUCCESS)
		{
			return nullptr;
		}

		return new VulkanDescriptorSet(device_, descriptorSet, vulkanLayout);  // ✅ Layout 전달
	}

	// ========================================
	// VulkanDescriptorSet
	// ========================================

	VulkanDescriptorSet::VulkanDescriptorSet(VkDevice device, VkDescriptorSet descriptorSet, VulkanDescriptorSetLayout* layout)
		: device_(device), descriptorSet_(descriptorSet), layout_(layout)  // ✅ Layout 저장
	{
	}

	VulkanDescriptorSet::~VulkanDescriptorSet()
	{
		// 디스크립터 셋은 풀이 해제되면 자동으로 해제됨
	}

	void VulkanDescriptorSet::updateBuffer(uint32_t binding, RHIBuffer* buffer, RHIDeviceSize offset, RHIDeviceSize range)
	{
		auto* vulkanBuffer = static_cast<VulkanBuffer*>(buffer);

		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = vulkanBuffer->getVkBuffer();
		bufferInfo.offset = offset;
		bufferInfo.range = range > 0 ? range : vulkanBuffer->getSize();

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSet_;
		descriptorWrite.dstBinding = binding;
		descriptorWrite.dstArrayElement = 0;
		
		// ✅ Layout에서 descriptor type 조회
		VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;  // 기본값
		if (layout_)
		{
			const auto& bindings = layout_->getBindings();
			for (const auto& b : bindings)
			{
				if (b.binding == binding)
				{
					descriptorType = b.descriptorType;
					break;
				}
			}
		}
		
		descriptorWrite.descriptorType = descriptorType;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(device_, 1, &descriptorWrite, 0, nullptr);
	}

	void VulkanDescriptorSet::updateImage(uint32_t binding, RHIImageView* imageView, RHISampler* sampler)
	{
		auto* vulkanImageView = static_cast<VulkanImageView*>(imageView);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = vulkanImageView->getVkImageView();
		
		// ✅ Sampler 변환
		if (sampler)
		{
			auto* vulkanSampler = static_cast<VulkanSampler*>(sampler);
			imageInfo.sampler = vulkanSampler->getVkSampler();
		}
		else
		{
			imageInfo.sampler = VK_NULL_HANDLE;
		}

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSet_;
		descriptorWrite.dstBinding = binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(device_, 1, &descriptorWrite, 0, nullptr);
	}

	// ========================================
	// ✅ 자동 풀 관리 메서드 구현
	// ========================================

	bool VulkanDescriptorPool::canAllocate(const std::vector<VkDescriptorSetLayoutBinding>& bindings, uint32_t setCount) const
	{
		// Set 개수 확인
		if (remainingSets_ < setCount)
		{
			return false;
		}

		// 각 Descriptor 타입별 개수 계산
		std::unordered_map<VkDescriptorType, uint32_t> requiredDescriptors;
		for (const auto& binding : bindings)
		{
			requiredDescriptors[binding.descriptorType] += binding.descriptorCount * setCount;
		}

		// 남은 Descriptor 개수 확인
		for (const auto& [type, required] : requiredDescriptors)
		{
			auto it = remainingDescriptors_.find(type);
			if (it == remainingDescriptors_.end() || it->second < required)
			{
				return false;
			}
		}

		return true;
	}

	void VulkanDescriptorPool::updateCapacity(const std::vector<VkDescriptorSetLayoutBinding>& bindings, uint32_t setCount)
	{
		// Set 개수 감소
		remainingSets_ -= setCount;

		// 각 Descriptor 타입별 개수 감소
		for (const auto& binding : bindings)
		{
			uint32_t totalUsed = binding.descriptorCount * setCount;
			remainingDescriptors_[binding.descriptorType] -= totalUsed;
		}
	}

} // namespace BinRenderer::Vulkan
