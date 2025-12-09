#pragma once

#include "../Core/RHIType.h"
#include "RHIImage.h"

namespace BinRenderer
{
	/**
	 * @brief 렌더 타겟 (Framebuffer + RenderPass 조합)
	 */
	class RHIRenderTarget
	{
	public:
		virtual ~RHIRenderTarget() = default;

		virtual uint32_t getWidth() const = 0;
		virtual uint32_t getHeight() const = 0;
		virtual uint32_t getColorAttachmentCount() const = 0;
		virtual RHIImageView* getColorAttachment(uint32_t index) const = 0;
		virtual RHIImageView* getDepthStencilAttachment() const = 0;
	};

} // namespace BinRenderer
