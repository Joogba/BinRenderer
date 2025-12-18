#pragma once

#include "VulkanDescriptor.h"
#include <memory>
#include <vector>

namespace BinRenderer::Vulkan
{
	/**
	 * @brief 자동 Descriptor Pool 관리자
	 * 
	 * 풀이 가득 차면 자동으로 새 풀을 생성하고 할당
	 * 여러 풀을 관리하며 최적의 풀에서 할당
	 */
	class VulkanDescriptorAllocator
	{
	public:
		VulkanDescriptorAllocator(VkDevice device);
		~VulkanDescriptorAllocator();

		/**
		 * @brief Descriptor Set 할당
		 * @param layout Descriptor set layout
		 * @param bindings Layout bindings (용량 체크용)
		 * @return 할당된 descriptor set
		 */
		VkDescriptorSet allocate(
			VkDescriptorSetLayout layout,
			const std::vector<VkDescriptorSetLayoutBinding>& bindings);

		/**
		 * @brief 모든 풀 리셋
		 */
		void resetAll();

		/**
		 * @brief 통계 출력
		 */
		void printStatistics() const;

		/**
		 * @brief 기본 풀 크기 설정
		 */
		struct PoolSizes
		{
			uint32_t uniformBuffer = 100;
			uint32_t storageBuffer = 50;
			uint32_t combinedImageSampler = 100;
			uint32_t storageImage = 50;
			uint32_t inputAttachment = 10;
			uint32_t maxSets = 100;
		};

		void setDefaultPoolSizes(const PoolSizes& sizes) { defaultPoolSizes_ = sizes; }

	private:
		VkDevice device_;
		std::vector<std::unique_ptr<VulkanDescriptorPool>> pools_;
		PoolSizes defaultPoolSizes_;

		/**
		 * @brief 새 풀 생성
		 * @param bindings 필요한 바인딩 (크기 결정용)
		 * @return 생성된 풀
		 */
		VulkanDescriptorPool* createNewPool(const std::vector<VkDescriptorSetLayoutBinding>& bindings);

		/**
		 * @brief 바인딩에서 필요한 Pool Sizes 계산
		 */
		std::vector<VkDescriptorPoolSize> calculatePoolSizes(
			const std::vector<VkDescriptorSetLayoutBinding>& bindings) const;

		/**
		 * @brief 기본 Pool Sizes 생성
		 */
		std::vector<VkDescriptorPoolSize> createDefaultPoolSizes() const;
	};

} // namespace BinRenderer::Vulkan
