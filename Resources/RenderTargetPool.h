#pragma once

#include "Core/Handle.h"
#include "TextureRegistry.h"

namespace BinRenderer {
    class RenderTargetPool {
    public:
        virtual ~RenderTargetPool() = default;
        virtual TextureHandle Acquire(const TextureDesc& desc) = 0;
        virtual void Release(TextureHandle handle) = 0;
        virtual void Reset() = 0;
    };
}