#pragma once

#include <vulkan/vulkan.h>
#include <spirv_reflect.h>
#include <string>
#include <vector>
#include <map>

namespace BinRenderer::Vulkan
{
	/**
	 * @brief SPIRV 셰이더 바인딩 정보
	 */
	struct ShaderBindingInfo
	{
		std::string name;        // Binding 리소스 이름
		uint32_t set = 0;  // Descriptor set index
		uint32_t binding = 0;            // Binding index
		VkDescriptorType descriptorType;     // Descriptor 타입
		uint32_t descriptorCount = 1;        // Array 크기
		VkShaderStageFlags stageFlags = 0;   // 사용되는 셰이더 스테이지
		
		// 이미지 전용
		VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkAccessFlags2 accessFlags = VK_ACCESS_2_NONE;
		VkPipelineStageFlags2 stageFlags2 = VK_PIPELINE_STAGE_2_NONE;
		bool writeOnly = false;
	};

	/**
	 * @brief SPIRV 셰이더 Push Constants 정보
	 */
	struct ShaderPushConstantInfo
	{
		std::string name;
		uint32_t offset = 0;
		uint32_t size = 0;
		VkShaderStageFlags stageFlags = 0;
	};

	/**
	 * @brief SPIRV 셰이더 Vertex Input 정보
	 */
	struct ShaderVertexInputInfo
	{
		uint32_t location = 0;
		VkFormat format = VK_FORMAT_UNDEFINED;
		uint32_t offset = 0;
		std::string name;
	};

	/**
	 * @brief SPIRV 셰이더 리플렉션 결과
	 */
	struct ShaderReflectionData
	{
		// Descriptor bindings (set별로 정리)
		std::map<uint32_t, std::vector<ShaderBindingInfo>> bindings;
		
		// Push constants
		std::vector<ShaderPushConstantInfo> pushConstants;
		
		// Vertex inputs (Vertex Shader만)
		std::vector<ShaderVertexInputInfo> vertexInputs;
		
		// Compute workgroup size (Compute Shader만)
		uint32_t workgroupSizeX = 1;
		uint32_t workgroupSizeY = 1;
		uint32_t workgroupSizeZ = 1;
	};

	/**
	 * @brief SPIRV-Reflect를 사용한 셰이더 리플렉션 유틸리티
	 */
	class VulkanShaderReflection
	{
	public:
		/**
		 * @brief SPIRV 바이너리에서 리플렉션 정보 추출
		 * @param spirvCode SPIRV 바이너리 데이터
		 * @param codeSize 바이너리 크기 (bytes)
		 * @param shaderStage 셰이더 스테이지
		 * @return 리플렉션 결과
		 */
		static ShaderReflectionData reflect(
			const uint32_t* spirvCode,
			size_t codeSize,
			VkShaderStageFlagBits shaderStage);

		/**
		 * @brief 여러 셰이더의 바인딩을 병합 (파이프라인용)
		 * @param reflections 리플렉션 데이터 배열
		 * @return 병합된 바인딩 맵
		 */
		static std::map<uint32_t, std::vector<ShaderBindingInfo>> mergeBindings(
			const std::vector<ShaderReflectionData>& reflections);

		/**
		 * @brief Descriptor Set Layout Bindings 생성
		 * @param bindings 바인딩 정보 맵 (set -> bindings)
		 * @param setIndex Set index
		 * @return VkDescriptorSetLayoutBinding 배열
		 */
		static std::vector<VkDescriptorSetLayoutBinding> createLayoutBindings(
			const std::map<uint32_t, std::vector<ShaderBindingInfo>>& bindings,
			uint32_t setIndex);

		/**
		 * @brief Push Constant Range 생성
		 * @param reflections 리플렉션 데이터 배열
		 * @return VkPushConstantRange 배열
		 */
		static std::vector<VkPushConstantRange> createPushConstantRanges(
			const std::vector<ShaderReflectionData>& reflections);

		/**
		 * @brief Vertex Input Attribute Descriptions 생성
		 * @param reflection Vertex shader 리플렉션 데이터
		 * @return VkVertexInputAttributeDescription 배열
		 */
		static std::vector<VkVertexInputAttributeDescription> createVertexInputAttributes(
			const ShaderReflectionData& reflection);

	private:
		/**
		 * @brief Descriptor binding 정보 추출
		 */
		static void extractBindings(
			const SpvReflectDescriptorBinding* binding,
			VkShaderStageFlagBits shaderStage,
			ShaderBindingInfo& outInfo);

		/**
		 * @brief Image layout/access 자동 결정
		 */
		static void determineImageAccess(
			const SpvReflectDescriptorBinding* binding,
			VkShaderStageFlagBits shaderStage,
			ShaderBindingInfo& outInfo);

		/**
		 * @brief Pipeline stage flags 결정
		 */
		static VkPipelineStageFlags2 getStageFlagsFromShaderStage(VkShaderStageFlagBits stage);
	};

} // namespace BinRenderer::Vulkan
