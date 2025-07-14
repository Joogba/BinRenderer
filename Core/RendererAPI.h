#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

#include "Handle.h"
#include "RenderStates.h"
#include "RenderEnums.h"
#include "DrawCommand.h"

namespace BinRenderer {


	

	struct InitParams {
		void* windowHandle;
		uint32_t width;
		uint32_t height;
	};

	

	

	class RendererAPI {
	public:
		virtual ~RendererAPI() noexcept = default;

		virtual bool Init(const InitParams& params) = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;
		virtual void Present() = 0;

		virtual TextureHandle             CreateTexture(const TextureDesc& desc) = 0;
		virtual RenderTargetViewHandle    CreateRTV(TextureHandle tex) = 0;
		virtual ShaderResourceViewHandle  CreateSRV(TextureHandle tex) = 0;
		virtual DepthStencilViewHandle    CreateDSV(TextureHandle tex) = 0;
		virtual PSOHandle                 CreatePipelineState(const PSODesc& desc) = 0;
		virtual SamplerHandle             CreateSampler(const SamplerDesc& desc) = 0;

		// 렌더 패스 바인딩
		virtual void BindPipelineState(PSOHandle pso) = 0;
		virtual void BindRenderTargets(const RenderTargetViewHandle* rtvs, size_t count, DepthStencilViewHandle dsv) = 0;
		virtual void ClearRenderTargets(uint32_t flags, uint32_t clearColor, float clearDepth, uint8_t clearStencil) = 0;

		// 셰이더 리소스 바인딩
		virtual void BindShaderResource(uint32_t slot, ShaderResourceViewHandle srv) = 0;
		virtual void BindSampler(SamplerHandle sampler, uint32_t slot) = 0;

		// Draw 호출
		virtual void EnqueueDraw(const DrawCommand& cmd) = 0;

		virtual void DrawSingle(const DrawCommand& cmd) = 0;
		virtual void DrawInstanced(const DrawCommand& cmd, const std::vector<glm::mat4>& transforms, int count) = 0;

		virtual void ExecuteDrawQueue() = 0;
		virtual void BindFullScreenQuad() = 0;
		virtual void DrawFullScreenQuad() = 0;

		// 이름 기반 리소스 조회 (PassResources용)
		virtual RenderTargetViewHandle   GetRTVByName(const char* name) const = 0;
		virtual DepthStencilViewHandle   GetDSVByName(const char* name) const = 0;
		virtual ShaderResourceViewHandle GetSRVByName(const char* name) const = 0;
	};

	RendererAPI* CreateD3D11Renderer();
	void DestroyRenderer(RendererAPI* renderer);

} // namespace BinRenderer