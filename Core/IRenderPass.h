#pragma once
#include "RendererAPI.h"
#include "RenderGraphBuilder.h"
#include "PassResources.h"

namespace BinRenderer
{

    class IRenderPass
    {
    public:
        virtual ~IRenderPass() = default;

        virtual bool Initialize(RendererAPI* rhi) { return true; }

        virtual void Declare(RenderGraphBuilder& builder) = 0;

        virtual void Execute(RendererAPI* rhi, const PassResources& res) = 0;
    };

}