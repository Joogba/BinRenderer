#pragma once
#include <string>
#include <vector>
#include <map>
#include <cstdint>

namespace BinRenderer
{
	// ========================================
	// Platform-Independent Shader Types
	// ========================================

	/**
	 * @brief 셰이더 스테이지 타입
	 */
	enum class RHIShaderStage : uint32_t
	{
		Vertex = 0x00000001,
		TessellationControl = 0x00000002,
		TessellationEvaluation = 0x00000004,
		Geometry = 0x00000008,
		Fragment = 0x00000010,
		Compute = 0x00000020,
		AllGraphics = 0x0000001F,
		All = 0x7FFFFFFF
	};

	inline RHIShaderStage operator|(RHIShaderStage a, RHIShaderStage b)
	{
		return static_cast<RHIShaderStage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
	}

	inline RHIShaderStage operator&(RHIShaderStage a, RHIShaderStage b)
	{
		return static_cast<RHIShaderStage>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
	}

	/**
	 * @brief Descriptor 타입
	 */
	enum class RHIDescriptorType : uint32_t
	{
		Sampler = 0,
		CombinedImageSampler = 1,
		SampledImage = 2,
		StorageImage = 3,
		UniformTexelBuffer = 4,
		StorageTexelBuffer = 5,
		UniformBuffer = 6,
		StorageBuffer = 7,
		UniformBufferDynamic = 8,
		StorageBufferDynamic = 9,
		InputAttachment = 10,
		AccelerationStructure = 11  // Ray Tracing
	};

	/**
	 * @brief 이미지 레이아웃
	 */
	enum class RHIImageLayout : uint32_t
	{
		Undefined = 0,
		General = 1,
		ColorAttachment = 2,
		DepthStencilAttachment = 3,
		DepthStencilReadOnly = 4,
		ShaderReadOnly = 5,
		TransferSrc = 6,
		TransferDst = 7,
		Preinitialized = 8,
		PresentSrc = 9
	};

	/**
	 * @brief 버텍스 인풋 포맷
	 */
	enum class RHIVertexFormat : uint32_t
	{
		Undefined = 0,
		// Float formats
		R32_Float = 100,
		R32G32_Float = 103,
		R32G32B32_Float = 106,
		R32G32B32A32_Float = 109,
		// Int formats
		R32_Sint = 99,
		R32G32_Sint = 102,
		R32G32B32_Sint = 105,
		R32G32B32A32_Sint = 108,
		// Uint formats
		R32_Uint = 98,
		R32G32_Uint = 101,
		R32G32B32_Uint = 104,
		R32G32B32A32_Uint = 107,
		// Normalized formats
		R8G8B8A8_Unorm = 37,
		R8G8B8A8_Snorm = 38
	};

	/**
	 * @brief 접근 플래그
	 */
	enum class RHIAccessFlags : uint64_t
	{
		None = 0,
		IndirectCommandRead = 0x00000001,
		IndexRead = 0x00000002,
		VertexAttributeRead = 0x00000004,
		UniformRead = 0x00000008,
		InputAttachmentRead = 0x00000010,
		ShaderRead = 0x00000020,
		ShaderWrite = 0x00000040,
		ColorAttachmentRead = 0x00000080,
		ColorAttachmentWrite = 0x00000100,
		DepthStencilAttachmentRead = 0x00000200,
		DepthStencilAttachmentWrite = 0x00000400,
		TransferRead = 0x00000800,
		TransferWrite = 0x00001000,
		HostRead = 0x00002000,
		HostWrite = 0x00004000,
		MemoryRead = 0x00008000,
		MemoryWrite = 0x00010000
	};

	inline RHIAccessFlags operator|(RHIAccessFlags a, RHIAccessFlags b)
	{
		return static_cast<RHIAccessFlags>(static_cast<uint64_t>(a) | static_cast<uint64_t>(b));
	}

	// ========================================
	// Shader Reflection Data Structures
	// ========================================

	/**
	 * @brief SPIRV 셰이더 바인딩 정보
	 */
	struct ShaderBindingInfo
	{
		std::string name;                           // Binding 리소스 이름 (예: "ubo", "samplerColor")
		uint32_t set = 0;                          // Descriptor set index
		uint32_t binding = 0;                      // Binding index
		RHIDescriptorType descriptorType = RHIDescriptorType::UniformBuffer;
		uint32_t descriptorCount = 1;              // Array 크기 (bindless 등)
		RHIShaderStage stageFlags = RHIShaderStage::Fragment;

		// 이미지 전용 정보
		RHIImageLayout imageLayout = RHIImageLayout::Undefined;
		RHIAccessFlags accessFlags = RHIAccessFlags::None;
		bool writeOnly = false;

		// 버퍼 전용 정보
		uint32_t bufferSize = 0;                   // Uniform/Storage buffer 크기
	};

	/**
	 * @brief SPIRV 셰이더 Push Constants 정보
	 */
	struct ShaderPushConstantInfo
	{
		std::string name;                          // Push constant 블록 이름
		uint32_t offset = 0;                       // 오프셋 (바이트)
		uint32_t size = 0;                         // 크기 (바이트)
		RHIShaderStage stageFlags = RHIShaderStage::Vertex;
	};

	/**
	 * @brief SPIRV 셰이더 Vertex Input 정보
	 */
	struct ShaderVertexInputInfo
	{
		uint32_t location = 0;                     // Input location
		RHIVertexFormat format = RHIVertexFormat::Undefined;
		uint32_t offset = 0;                       // Vertex buffer 내 오프셋
		std::string name;                          // 변수 이름 (예: "inPosition")
		std::string semanticName;                  // Semantic (예: "POSITION", "TEXCOORD")
	};

	/**
	 * @brief 셰이더 리소스 사용량 통계
	 */
	struct ShaderResourceUsage
	{
		uint32_t numUniformBuffers = 0;
		uint32_t numStorageBuffers = 0;
		uint32_t numSampledImages = 0;
		uint32_t numStorageImages = 0;
		uint32_t numSamplers = 0;
		uint32_t numInputAttachments = 0;
		uint32_t totalDescriptors = 0;
	};

	/**
	 * @brief SPIRV 셰이더 리플렉션 결과
	 */
	struct ShaderReflectionData
	{
		// Descriptor bindings (set별로 정리)
		// Key: set index, Value: bindings in that set
		std::map<uint32_t, std::vector<ShaderBindingInfo>> bindings;

		// Push constants
		std::vector<ShaderPushConstantInfo> pushConstants;

		// Vertex inputs (Vertex Shader만)
		std::vector<ShaderVertexInputInfo> vertexInputs;

		// Compute workgroup size (Compute Shader만)
		uint32_t workgroupSizeX = 1;
		uint32_t workgroupSizeY = 1;
		uint32_t workgroupSizeZ = 1;

		// 리소스 사용량 통계
		ShaderResourceUsage resourceUsage;

		// 셰이더 스테이지
		RHIShaderStage stage = RHIShaderStage::Fragment;

		// 셰이더 엔트리 포인트
		std::string entryPoint = "main";

		/**
		 * @brief 특정 set의 바인딩 정보 가져오기
		 */
		const std::vector<ShaderBindingInfo>* getBindings(uint32_t setIndex) const
		{
			auto it = bindings.find(setIndex);
			return (it != bindings.end()) ? &it->second : nullptr;
		}

		/**
		 * @brief 특정 set, binding의 정보 가져오기
		 */
		const ShaderBindingInfo* getBinding(uint32_t setIndex, uint32_t bindingIndex) const
		{
			auto it = bindings.find(setIndex);
			if (it == bindings.end())
				return nullptr;

			for (const auto& binding : it->second)
			{
				if (binding.binding == bindingIndex)
					return &binding;
			}
			return nullptr;
		}

		/**
		 * @brief 리소스 사용량 계산
		 */
		void calculateResourceUsage()
		{
			resourceUsage = {};

			for (const auto& [setIdx, setBindings] : bindings)
			{
				for (const auto& binding : setBindings)
				{
					switch (binding.descriptorType)
					{
					case RHIDescriptorType::UniformBuffer:
					case RHIDescriptorType::UniformBufferDynamic:
						resourceUsage.numUniformBuffers += binding.descriptorCount;
						break;
					case RHIDescriptorType::StorageBuffer:
					case RHIDescriptorType::StorageBufferDynamic:
						resourceUsage.numStorageBuffers += binding.descriptorCount;
						break;
					case RHIDescriptorType::SampledImage:
					case RHIDescriptorType::CombinedImageSampler:
						resourceUsage.numSampledImages += binding.descriptorCount;
						break;
					case RHIDescriptorType::StorageImage:
						resourceUsage.numStorageImages += binding.descriptorCount;
						break;
					case RHIDescriptorType::Sampler:
						resourceUsage.numSamplers += binding.descriptorCount;
						break;
					case RHIDescriptorType::InputAttachment:
						resourceUsage.numInputAttachments += binding.descriptorCount;
						break;
					default:
						break;
					}
					resourceUsage.totalDescriptors += binding.descriptorCount;
				}
			}
		}

		/**
		 * @brief 디버그 출력
		 */
		void printDebugInfo() const;
	};

	// ========================================
	// Shader Reflection Interface
	// ========================================

	/**
	 * @brief 셰이더 리플렉션 추상 인터페이스
	 * 
	 * SPIR-V Reflect, DXC, SPIRV-Cross 등 다양한 백엔드에서 구현 가능
	 * 
	 * 사용 예시:
	 * @code
	 * auto reflection = std::make_unique<VulkanShaderReflection>(spirvCode);
	 * reflection->reflect();
	 * const auto& data = reflection->getReflectionData();
	 * @endcode
	 */
	class RHIShaderReflection
	{
	public:
		virtual ~RHIShaderReflection() = default;

		/**
		 * @brief 셰이더 바이너리에서 리플렉션 데이터 추출
		 * @return 성공 여부
		 */
		virtual bool reflect() = 0;

		/**
		 * @brief 리플렉션 결과 데이터 반환
		 */
		virtual const ShaderReflectionData& getReflectionData() const = 0;

		/**
		 * @brief 셰이더 스테이지 반환
		 */
		virtual RHIShaderStage getShaderStage() const = 0;

		/**
		 * @brief 엔트리 포인트 이름 반환
		 */
		virtual const std::string& getEntryPoint() const = 0;

		/**
		 * @brief 리플렉션 데이터 유효성 검증
		 */
		virtual bool validate() const = 0;

		/**
		 * @brief 특정 descriptor set layout 정보 반환
		 * @param setIndex Descriptor set index
		 * @return Bindings in that set (nullptr if not found)
		 */
		virtual const std::vector<ShaderBindingInfo>* getDescriptorSetBindings(uint32_t setIndex) const = 0;

		/**
		 * @brief Push constant 정보 반환
		 */
		virtual const std::vector<ShaderPushConstantInfo>& getPushConstants() const = 0;

		/**
		 * @brief Vertex input 정보 반환 (Vertex shader만)
		 */
		virtual const std::vector<ShaderVertexInputInfo>& getVertexInputs() const = 0;

		/**
		 * @brief Compute shader workgroup size 반환 (Compute shader만)
		 */
		virtual void getComputeWorkgroupSize(uint32_t& x, uint32_t& y, uint32_t& z) const = 0;
	};

} // namespace BinRenderer