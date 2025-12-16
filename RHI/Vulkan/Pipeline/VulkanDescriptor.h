#pragma once

#include "../../Pipeline/RHIDescriptor.h"
#include <vulkan/vulkan.h>
#include <vector>

namespace BinRenderer::Vulkan
{
	/**
	 * @brief Vulkan 디스크립터 셋 레이아웃
*/
	class VulkanDescriptorSetLayout : public RHIDescriptorSetLayout
	{
	public:
		VulkanDescriptorSetLayout(VkDevice device);
		~VulkanDescriptorSetLayout() override;

		bool create(const std::vector<VkDescriptorSetLayoutBinding>& bindings);
		void destroy();

		// RHIDescriptorSetLayout 인터페이스 구현
		uint32_t getBindingCount() const override { return bindingCount_; }

		// Vulkan 네이티브 접근
		VkDescriptorSetLayout getVkDescriptorSetLayout() const { return layout_; }

	private:
		VkDevice device_;
		VkDescriptorSetLayout layout_ = VK_NULL_HANDLE;
		uint32_t bindingCount_ = 0;
	};

	/**
	 * @brief Vulkan 디스크립터 풀
	 */
	class VulkanDescriptorPool : public RHIDescriptorPool
	{
	public:
		VulkanDescriptorPool(VkDevice device);
		~VulkanDescriptorPool() override;

		bool create(uint32_t maxSets, const std::vector<VkDescriptorPoolSize>& poolSizes);
		void destroy();

		// RHIDescriptorPool 인터페이스 구현
		void reset() override;
		RHIDescriptorSet* allocateDescriptorSet(RHIDescriptorSetLayout* layout) override;

		// Vulkan 네이티브 접근
		VkDescriptorPool getVkDescriptorPool() const { return pool_; }

	private:
		VkDevice device_;
		VkDescriptorPool pool_ = VK_NULL_HANDLE;
	};

	/**
	 * @brief Vulkan 디스크립터 셋
  */
	class VulkanDescriptorSet : public RHIDescriptorSet
	{
	public:
		VulkanDescriptorSet(VkDevice device, VkDescriptorSet descriptorSet);
		~VulkanDescriptorSet() override;

		// RHIDescriptorSet 인터페이스 구현
		void updateBuffer(uint32_t binding, RHIBuffer* buffer, RHIDeviceSize offset = 0, RHIDeviceSize range = 0) override;
		void updateImage(uint32_t binding, RHIImageView* imageView, class RHISampler* sampler = nullptr) override;

		// Vulkan 네이티브 접근
		VkDescriptorSet getVkDescriptorSet() const { return descriptorSet_; }

	private:
		VkDevice device_;
		VkDescriptorSet descriptorSet_;
	};

} // namespace BinRenderer::Vulkan
