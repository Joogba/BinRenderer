#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace BinRenderer::Vulkan
{
	/**
	 * @brief Vulkan Image Layout 전환 유틸리티
	 * 
	 * VkImageMemoryBarrier2를 사용한 최신 동기화 API
	 * Vulkan 1.3+ 또는 VK_KHR_synchronization2 확장 필요
	 */
	class VulkanBarrier
	{
	public:
		VulkanBarrier() = default;
		VulkanBarrier(VkImage image, VkFormat format, uint32_t mipLevels = 1, uint32_t arrayLayers = 1);

		// Move semantics
		VulkanBarrier(VulkanBarrier&& other) noexcept;
		VulkanBarrier& operator=(VulkanBarrier&& other) noexcept;

		// Delete copy semantics
		VulkanBarrier(const VulkanBarrier&) = delete;
		VulkanBarrier& operator=(const VulkanBarrier&) = delete;

		/**
		 * @brief Image 정보 업데이트
		 */
		void setImage(VkImage image, VkFormat format, uint32_t mipLevels = 1, uint32_t arrayLayers = 1);

		/**
		 * @brief Layout 전환 수행
		 * @param cmd Command buffer
		 * @param newLayout 새로운 layout
		 * @param newAccess 새로운 access flags
		 * @param newStage 새로운 pipeline stage
		 * @param baseMipLevel 시작 mip level
		 * @param levelCount Mip level 개수 (VK_REMAINING_MIP_LEVELS 사용 가능)
		 * @param baseArrayLayer 시작 array layer
		 * @param layerCount Array layer 개수 (VK_REMAINING_ARRAY_LAYERS 사용 가능)
		 */
		void transitionLayout(
			VkCommandBuffer cmd,
			VkImageLayout newLayout,
			VkAccessFlags2 newAccess,
			VkPipelineStageFlags2 newStage,
			uint32_t baseMipLevel = 0,
			uint32_t levelCount = VK_REMAINING_MIP_LEVELS,
			uint32_t baseArrayLayer = 0,
			uint32_t layerCount = VK_REMAINING_ARRAY_LAYERS);

		/**
		 * @brief Barrier 준비 (배치 전환용)
		 * @return VkImageMemoryBarrier2 구조체
		 */
		VkImageMemoryBarrier2 prepareBarrier(
			VkImageLayout targetLayout,
			VkAccessFlags2 targetAccess,
			VkPipelineStageFlags2 targetStage);

		// Getters
		VkImageLayout getCurrentLayout() const { return currentLayout_; }
		VkAccessFlags2 getCurrentAccess() const { return currentAccess_; }
		VkPipelineStageFlags2 getCurrentStage() const { return currentStage_; }
		VkFormat getFormat() const { return format_; }
		VkImage getImage() const { return image_; }

		// ========================================
		// 편의 메서드 (자주 사용하는 전환)
		// ========================================

		/**
		 * @brief UNDEFINED → TRANSFER_DST (복사/클리어 대상)
		 */
		void transitionToTransferDst(VkCommandBuffer cmd);

		/**
		 * @brief TRANSFER_DST → SHADER_READ_ONLY (셰이더 읽기)
		 */
		void transitionToShaderReadOnly(VkCommandBuffer cmd);

		/**
		 * @brief UNDEFINED → COLOR_ATTACHMENT (렌더 타겟)
		 */
		void transitionToColorAttachment(VkCommandBuffer cmd);

		/**
		 * @brief UNDEFINED → DEPTH_STENCIL_ATTACHMENT (Depth buffer)
		 */
		void transitionToDepthAttachment(VkCommandBuffer cmd);

		/**
		 * @brief COLOR_ATTACHMENT → SHADER_READ_ONLY (렌더링 후 셰이더 읽기)
		 */
		void transitionColorToShaderRead(VkCommandBuffer cmd);

		/**
		 * @brief SHADER_READ_ONLY → COLOR_ATTACHMENT (셰이더 읽기 후 렌더링)
		 */
		void transitionShaderReadToColor(VkCommandBuffer cmd);

		/**
		 * @brief COLOR_ATTACHMENT → PRESENT_SRC (Present용)
		 */
		void transitionColorToPresent(VkCommandBuffer cmd);

	private:
		VkImage image_ = VK_NULL_HANDLE;
		VkFormat format_ = VK_FORMAT_UNDEFINED;
		uint32_t mipLevels_ = 1;
		uint32_t arrayLayers_ = 1;

		VkImageLayout currentLayout_ = VK_IMAGE_LAYOUT_UNDEFINED;
		VkAccessFlags2 currentAccess_ = VK_ACCESS_2_NONE;
		VkPipelineStageFlags2 currentStage_ = VK_PIPELINE_STAGE_2_NONE;

		/**
		 * @brief Format에서 Image aspect 자동 추출
		 */
		VkImageAspectFlags getAspectMask() const;

		/**
		 * @brief Layout 전환이 유효한지 검증
		 */
		bool isValidTransition(VkImageLayout oldLayout, VkImageLayout newLayout) const;
	};

	/**
	 * @brief 전역 헬퍼 함수들
	 */
	namespace BarrierHelpers
	{
		/**
		 * @brief 간단한 Layout 전환 (상태 추적 없음)
		 */
		void transitionImageLayout(
			VkCommandBuffer cmd,
			VkImage image,
			VkFormat format,
			VkImageLayout oldLayout,
			VkImageLayout newLayout,
			uint32_t mipLevels = 1,
			uint32_t arrayLayers = 1);

		/**
		 * @brief Image aspect mask 자동 결정
		 */
		VkImageAspectFlags getImageAspect(VkFormat format);

		/**
		 * @brief Layout과 Access mask 자동 매칭
		 */
		struct LayoutAccessInfo
		{
			VkAccessFlags2 accessMask;
			VkPipelineStageFlags2 stageMask;
		};

		LayoutAccessInfo getLayoutAccessInfo(VkImageLayout layout);
	}

} // namespace BinRenderer::Vulkan
