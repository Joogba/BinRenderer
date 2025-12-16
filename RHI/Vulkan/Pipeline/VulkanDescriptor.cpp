#include "VulkanDescriptor.h"
#include "../Resources/VulkanBuffer.h"
#include "../Resources/VulkanImage.h"
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
		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = maxSets;

		if (vkCreateDescriptorPool(device_, &poolInfo, nullptr, &pool_) != VK_SUCCESS)
		{
			return false;
		}

		return true;
	}

	void VulkanDescriptorPool::destroy()
	{
		if (pool_ != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorPool(device_, pool_, nullptr);
			pool_ = VK_NULL_HANDLE;
		}
	}

	void VulkanDescriptorPool::reset()
	{
		if (pool_ != VK_NULL_HANDLE)
		{
			vkResetDescriptorPool(device_, pool_, 0);
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

		return new VulkanDescriptorSet(device_, descriptorSet);
	}

	// ========================================
	// VulkanDescriptorSet
	// ========================================

	VulkanDescriptorSet::VulkanDescriptorSet(VkDevice device, VkDescriptorSet descriptorSet)
		: device_(device), descriptorSet_(descriptorSet)
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
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
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
		imageInfo.sampler = VK_NULL_HANDLE; // TODO: Sampler 구현

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

} // namespace BinRenderer::Vulkan
