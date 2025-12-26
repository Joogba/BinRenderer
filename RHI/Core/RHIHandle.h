#pragma once

namespace BinRenderer
{
	struct RHIBufferTag {};
	struct RHIImageTag {};
	struct RHIImageViewTag {};
	struct RHIShaderTag {};
	struct RHISamplerTag {};
	struct RHITextureTag {};
	struct RHICommandBufferTag {};
	struct RHIPipelineTag {};
	struct RHIDescriptorSetTag {};
	struct RHIDescriptorSetLayoutTag {};
	struct RHIDescriptorPoolTag {};
	
	template<typename Tag>
	class RHIHandle
	{
	private:
		uint32_t id = 0; // 인덱스 20bit + Generation 12bit

	public:
		static const uint32_t kIndexBits = 20;
		static const uint32_t kGenerationBits = 12;
		static const uint32_t kIndexMask = (1 << kIndexBits) - 1;
		static const uint32_t kGenerationMask = (1 << kGenerationBits) - 1;

		RHIHandle() = default;
		RHIHandle(uint32_t index, uint32_t generation)
		{
			id = (generation << kIndexBits) | (index & kIndexMask);
		}

		uint32_t getIndex() const { return id & kIndexMask; }
		uint32_t getGeneration() const { return (id >> kIndexBits) & kGenerationMask; }
		bool isValid() const { return id != 0; }

		// 비교연산자
		bool operator==(const RHIHandle& other) const { return id == other.id; }
		bool operator!=(const RHIHandle& other) const { return id != other.id; }
		bool operator<(const RHIHandle& other) const { return id < other.id; }
	};

	// 핸들 alias
	using RHIBufferHandle = RHIHandle<RHIBufferTag>;
	using RHIImageHandle = RHIHandle<RHIImageTag>;
	using RHIImageViewHandle = RHIHandle<RHIImageViewTag>;
	using RHIShaderHandle = RHIHandle<RHIShaderTag>;
	using RHISamplerHandle = RHIHandle<RHISamplerTag>;
	using RHITextureHandle = RHIHandle<RHITextureTag>;
	using RHICommandBufferHandle = RHIHandle<RHICommandBufferTag>;
	using RHIPipelineHandle = RHIHandle<RHIPipelineTag>;
	using RHIDescriptorSetHandle = RHIHandle<RHIDescriptorSetTag>;
	using RHIDescriptorSetLayoutHandle = RHIHandle<RHIDescriptorSetLayoutTag>;
	using RHIDescriptorPoolHandle = RHIHandle<RHIDescriptorPoolTag>;


} // namespace BinRenderer