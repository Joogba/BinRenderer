#pragma once

#include "../../Resources/RHIShaderReflection.h"
#include <vulkan/vulkan.h>
#include <spirv-reflect/spirv_reflect.h>
#include <string>
#include <vector>
#include <map>

namespace BinRenderer::Vulkan
{
	/**
	 * @brief Vulkan SPIR-V 셰이더 리플렉션 구현
	 * 
	 * SPIRV-Reflect 라이브러리를 사용하여 SPIR-V 바이너리 분석
	 * 
	 * 사용 예시:
	 * @code
	 * std::vector<uint32_t> spirvCode = loadShader("shader.spv");
	 * auto reflection = std::make_unique<VulkanShaderReflection>(spirvCode.data(), spirvCode.size());
	 * if (reflection->reflect())
	 * {
	 *     const auto& data = reflection->getReflectionData();
	 *     data.printDebugInfo();
	 * }
	 * @endcode
	 */
	class VulkanShaderReflection : public RHIShaderReflection
	{
	public:
		/**
		 * @brief 생성자
		 * @param spirvCode SPIR-V 바이너리 데이터 (uint32_t 배열)
		 * @param codeSize 바이트 크기
		 */
		VulkanShaderReflection(const void* spirvCode, size_t codeSize);
		
		/**
		 * @brief 생성자 (std::vector 오버로드)
		 * @param spirvCode SPIR-V 바이너리 벡터
		 */
		explicit VulkanShaderReflection(const std::vector<uint32_t>& spirvCode);
		
		~VulkanShaderReflection() override;

		// RHIShaderReflection 인터페이스 구현
		bool reflect() override;
		const ShaderReflectionData& getReflectionData() const override;
		RHIShaderStage getShaderStage() const override;
		const std::string& getEntryPoint() const override;
		bool validate() const override;
		const std::vector<ShaderBindingInfo>* getDescriptorSetBindings(uint32_t setIndex) const override;
		const std::vector<ShaderPushConstantInfo>& getPushConstants() const override;
		const std::vector<ShaderVertexInputInfo>& getVertexInputs() const override;
		void getComputeWorkgroupSize(uint32_t& x, uint32_t& y, uint32_t& z) const override;

	private:
		// SPIR-V 바이너리
		const void* spirvCode_;
		size_t codeSize_;

		// SPIRV-Reflect 모듈
		SpvReflectShaderModule module_;
		bool moduleCreated_ = false;

		// 리플렉션 결과
		ShaderReflectionData reflectionData_;

		// 헬퍼 함수
		void reflectDescriptorBindings();
		void reflectPushConstants();
		void reflectVertexInputs();
		void reflectComputeWorkgroupSize();
		
		// 타입 변환 헬퍼
		static RHIDescriptorType convertDescriptorType(VkDescriptorType vkType);
		static RHIShaderStage convertShaderStage(VkShaderStageFlagBits vkStage);
		static RHIVertexFormat convertVertexFormat(VkFormat vkFormat);
		static RHIImageLayout convertImageLayout(VkImageLayout vkLayout);
		static RHIAccessFlags convertAccessFlags(VkAccessFlags2 vkAccess);
	};

} // namespace BinRenderer::Vulkan
