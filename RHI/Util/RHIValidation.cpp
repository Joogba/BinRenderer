#include "RHIValidation.h"

namespace BinRenderer
{
	bool RHIValidation::validateBufferCreateInfo(const RHIBufferCreateInfo& createInfo, std::string* outError)
	{
		if (createInfo.size == 0)
		{
			if (outError) *outError = "Buffer size must be greater than 0";
			return false;
		}

		if (createInfo.usage == 0)
		{
			if (outError) *outError = "Buffer usage flags must be specified";
			return false;
		}

		if (!isValidBufferUsage(createInfo.usage))
		{
			if (outError) *outError = "Invalid buffer usage flags";
			return false;
		}

		return true;
	}

	bool RHIValidation::validateImageCreateInfo(const RHIImageCreateInfo& createInfo, std::string* outError)
	{
		if (createInfo.width == 0 || createInfo.height == 0)
		{
			if (outError) *outError = "Image dimensions must be greater than 0";
			return false;
		}

		if (!isValidFormat(createInfo.format))
		{
			if (outError) *outError = "Invalid image format";
			return false;
		}

		if (createInfo.usage == 0)
		{
			if (outError) *outError = "Image usage flags must be specified";
			return false;
		}

		if (!isValidImageUsage(createInfo.usage))
		{
			if (outError) *outError = "Invalid image usage flags";
			return false;
		}

		if (createInfo.mipLevels == 0)
		{
			if (outError) *outError = "Mip levels must be at least 1";
			return false;
		}

		return true;
	}

	bool RHIValidation::validateShaderCreateInfo(const RHIShaderCreateInfo& createInfo, std::string* outError)
	{
		if (createInfo.code.empty())
		{
			if (outError) *outError = "Shader code is empty";
			return false;
		}

		if (createInfo.stage == 0)
		{
			if (outError) *outError = "Shader stage must be specified";
			return false;
		}

		if (!createInfo.entryPoint || createInfo.entryPoint[0] == '\0')
		{
			if (outError) *outError = "Shader entry point must be specified";
			return false;
		}

		return true;
	}

	bool RHIValidation::validatePipelineCreateInfo(const RHIPipelineCreateInfo& createInfo, std::string* outError)
	{
		if (createInfo.shaderStages.empty())
		{
			if (outError) *outError = "Pipeline must have at least one shader stage";
			return false;
		}

		// 셰이더 검증
		for (const auto shader : createInfo.shaderStages)
		{
			if (!shader)
			{
				if (outError) *outError = "Shader stage cannot be null";
				return false;
			}
		}

		return true;
	}

	bool RHIValidation::isValidFormat(RHIFormat format)
	{
		return format != RHI_FORMAT_UNDEFINED;
	}

	bool RHIValidation::isDepthFormat(RHIFormat format)
	{
		switch (format)
		{
		case RHI_FORMAT_D32_SFLOAT:
			// TODO: 더 많은 깊이 포맷 추가
			return true;
		default:
			return false;
		}
	}

	bool RHIValidation::isStencilFormat(RHIFormat format)
	{
		// TODO: 스텐실 포맷 구현
		return false;
	}

	bool RHIValidation::isValidBufferUsage(RHIBufferUsageFlags usage)
	{
		const RHIBufferUsageFlags validFlags =
			RHI_BUFFER_USAGE_VERTEX_BUFFER_BIT |
			RHI_BUFFER_USAGE_INDEX_BUFFER_BIT |
			RHI_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
			RHI_BUFFER_USAGE_STORAGE_BUFFER_BIT |
			RHI_BUFFER_USAGE_TRANSFER_SRC_BIT |
			RHI_BUFFER_USAGE_TRANSFER_DST_BIT;

		return (usage & ~validFlags) == 0;
	}

	bool RHIValidation::isValidImageUsage(RHIImageUsageFlags usage)
	{
		const RHIImageUsageFlags validFlags =
			RHI_IMAGE_USAGE_TRANSFER_SRC_BIT |
			RHI_IMAGE_USAGE_TRANSFER_DST_BIT |
			RHI_IMAGE_USAGE_SAMPLED_BIT |
			RHI_IMAGE_USAGE_STORAGE_BIT |
			RHI_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
			RHI_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

		return (usage & ~validFlags) == 0;
	}

} // namespace BinRenderer
