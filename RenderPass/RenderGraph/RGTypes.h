#pragma once

#include "../../RHI/Core/RHIType.h"
#include <string>
#include <vector>
#include <cstdint>

namespace BinRenderer
{
	// 리소스 핸들 (타입 안전성을 위한 템플릿)
	template<typename T>
	struct RenderGraphHandle
	{
		uint32_t index = UINT32_MAX;
		bool isValid() const { return index != UINT32_MAX; }
		
		bool operator==(const RenderGraphHandle& other) const { return index == other.index; }
		bool operator!=(const RenderGraphHandle& other) const { return index != other.index; }
	};

	// 리소스 타입별 핸들
	using RGTextureHandle = RenderGraphHandle<struct RGTexture>;
	using RGBufferHandle = RenderGraphHandle<struct RGBuffer>;

	// 리소스 접근 타입
	enum class RGResourceAccessType
	{
		Read,      // 읽기 전용
		Write,     // 쓰기 전용 (이전 내용 무시)
		ReadWrite  // 읽기/쓰기
	};

	// 텍스처 설명자
	struct RGTextureDesc
	{
		std::string name;
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t depth = 1;
		uint32_t mipLevels = 1;
		uint32_t arrayLayers = 1;
		RHIFormat format = RHI_FORMAT_UNDEFINED;
		RHISampleCountFlagBits samples = RHI_SAMPLE_COUNT_1_BIT;
		RHIImageUsageFlags usage = 0;
		bool isImported = false; // 외부 리소스 여부 (예: 스왑체인 이미지)
	};

	// 버퍼 설명자
	struct RGBufferDesc
	{
		std::string name;
		RHIDeviceSize size = 0;
		RHIBufferUsageFlags usage = 0;
		bool isImported = false; // 외부 리소스 여부
	};

	// 리소스 의존성 정보
	struct RGResourceDependency
	{
		RGTextureHandle texture;
		RGBufferHandle buffer;
		RGResourceAccessType accessType;
		bool isTexture = true; // true: texture, false: buffer
	};

} // namespace BinRenderer
