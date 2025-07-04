#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

#include "Handle.h"
#include "RenderStates.h"

namespace BinRenderer {


	// API 타입 확장 대비
	enum class APIType { None, D3D11, Vulkan, OpenGL };

	// 초기화 파라미터
	struct InitParams {
		void* windowHandle;
		uint32_t width;
		uint32_t height;
	};

	// Clear 플래그 (비트마스크)
	enum ClearFlags : uint32_t {
		ClearColor = 1 << 0,
		ClearDepth = 1 << 1,
		ClearStencil = 1 << 2,
	};

	// 텍스처 포맷
	enum class Format {
		RGBA32_FLOAT,
		R8G8B8A8_UNORM,
		R16G16B16A16_FLOAT,
		R32_FLOAT,
		DEPTH24_STENCIL8,
		// ... 필요시 추가
	};

	// 바인드 플래그
	enum BindFlags : uint32_t {
		Bind_None = 0,
		Bind_RenderTarget = 1 << 0,
		Bind_ShaderResource = 1 << 1,
		Bind_DepthStencil = 1 << 2,
	};

	// 텍스처 생성 파라미터
	struct TextureDesc {
		uint32_t width;
		uint32_t height;
		Format   format;
		uint32_t bindFlags;  // BindFlags 조합
	};

	// 파이프라인 상태 생성 파라미터
	struct PSODesc {
		std::string         name;
		std::string         vsFile;
		std::string         vsEntry;
		std::string         psFile;
		std::string         psEntry;
		std::vector<InputElementDesc> inputElements;
		RasterizerState     rasterizerState;
		DepthStencilState   depthStencilState;
		BlendState          blendState;
		float               blendFactor[4] = { 1,1,1,1 };
		UINT                stencilRef = 0;
		D3D11_PRIMITIVE_TOPOLOGY           primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		UINT                               sampleMask = 0xFFFFFFFF;

		uint32_t            numRenderTargets = 1;
		DXGI_FORMAT         rtvFormats[8];
		DXGI_FORMAT         dsvFormat;
	};

	// 샘플러 생성 파라미터
	struct SamplerDesc {
		FilterMode     filter;
		AddressMode    addressU;
		AddressMode    addressV;
		AddressMode    addressW;
		ComparisonFunc comparison;
		float          minLOD;
		float          maxLOD;
		float          mipLODBias;       // LOD 바이어스 (기본 0.0f)
		uint32_t       maxAnisotropy;    // 최대 이방성 필터링 레벨 (기본 1)
		float          borderColor[4];   // 보더 컬러 (Wrap=Border 모드 시 사용)
	};

	class RendererAPI {
	public:
		virtual ~RendererAPI() noexcept = default;

		// 초기화/종료
		virtual bool Init(const InitParams& params) = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;
		virtual void Present() = 0;

		// 리소스 생성
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