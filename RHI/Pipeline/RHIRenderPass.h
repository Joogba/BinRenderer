#pragma once

#include "../Core/RHIType.h"
#include "../Core/RHIResource.h"

namespace BinRenderer
{
    /**
     * @brief 렌더 패스
     */
    class RHIRenderPass : public RHIResource
    {
    public:
        virtual ~RHIRenderPass() = default;

        virtual uint32_t getAttachmentCount() const = 0;
        virtual uint32_t getSubpassCount() const = 0;
    };

} // namespace BinRenderer
