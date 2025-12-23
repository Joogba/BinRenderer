#pragma once

#include "../../Pipeline/RHIDescriptor.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>

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
		
		// ✅ Binding 정보 조회
		const std::vector<VkDescriptorSetLayoutBinding>& getBindings() const { return bindings_; }

	private:
		VkDevice device_;
		VkDescriptorSetLayout layout_ = VK_NULL_HANDLE;
		uint32_t bindingCount_ = 0;
		std::vector<VkDescriptorSetLayoutBinding> bindings_;  // ✅ Binding 정보 저장
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

		// ========================================
		// ✅ 자동 풀 관리 기능
		// ========================================

		/**
		 * @brief 현재 풀에서 할당 가능한지 확인
		 * @param bindings 필요한 바인딩
		 * @param setCount 필요한 set 개수
		 * @return 할당 가능 여부
		 */
		bool canAllocate(const std::vector<VkDescriptorSetLayoutBinding>& bindings, uint32_t setCount = 1) const;

		/**
		 * @brief 풀 용량 업데이트 (할당 후)
		 * @param bindings 할당된 바인딩
		 * @param setCount 할당된 set 개수
		 */
		void updateCapacity(const std::vector<VkDescriptorSetLayoutBinding>& bindings, uint32_t setCount = 1);

		/**
		 * @brief 남은 용량 확인
		 */
		uint32_t getRemainingSetCount() const { return remainingSets_; }
		const std::unordered_map<VkDescriptorType, uint32_t>& getRemainingDescriptors() const { return remainingDescriptors_; }

	private:
		VkDevice device_;
		VkDescriptorPool pool_ = VK_NULL_HANDLE;

		// ✅ 용량 추적
		uint32_t maxSets_ = 0;
		uint32_t remainingSets_ = 0;
		std::unordered_map<VkDescriptorType, uint32_t> totalDescriptors_;
		std::unordered_map<VkDescriptorType, uint32_t> remainingDescriptors_;
	};

	/**
	 * @brief Vulkan 디스크립터 셋
	 */
	class VulkanDescriptorSet : public RHIDescriptorSet
	{
	public:
		VulkanDescriptorSet(VkDevice device, VkDescriptorSet descriptorSet, VulkanDescriptorSetLayout* layout);  // ✅ Layout 추가
		~VulkanDescriptorSet() override;

		// RHIDescriptorSet 인터페이스 구현
		void updateBuffer(uint32_t binding, RHIBuffer* buffer, RHIDeviceSize offset = 0, RHIDeviceSize range = 0) override;
		void updateImage(uint32_t binding, RHIImageView* imageView, class RHISampler* sampler = nullptr) override;
		
		// Bindless descriptor array support
		void updateImageArray(uint32_t binding, uint32_t arrayIndex, RHIImageView* imageView, class RHISampler* sampler = nullptr) override;
		void updateImageArrayBatch(uint32_t binding, const std::vector<RHIImageView*>& imageViews, class RHISampler* sampler = nullptr) override;

		// Vulkan 네이티브 접근
		VkDescriptorSet getVkDescriptorSet() const { return descriptorSet_; }

	private:
		VkDevice device_;
		VkDescriptorSet descriptorSet_;
		VulkanDescriptorSetLayout* layout_;  // ✅ Layout 참조 저장
	};

} // namespace BinRenderer::Vulkan
