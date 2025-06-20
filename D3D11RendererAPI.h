#pragma once
#include "RendererAPI.h"
#include "DrawQueue.h"
#include "MeshRegistry.h"
#include "PSORegistry.h"
#include "TextureRegistry.h"
#include "SamplerRegistry.h"
#include "MaterialSystem.h"
#include "View.h"
#include "TransientBufferAllocator.h"

#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include <DirectXMath.h>
#include <unordered_map>
#include <cstdint> 



namespace BinRenderer
{

	class D3D11RendererAPI :
		public RendererAPI
	{
	public:
		D3D11RendererAPI();
		~D3D11RendererAPI() override;

		bool Init(const InitParams& params) override;
		void BeginFrame() override;
		void Submit() override;
		void Submit(const DrawCommand& cmd) override;
		void EndFrame() override;
		void Present() override;

		// Accessors for integration and testing
		ID3D11Device* GetDevice() const;
		ID3D11DeviceContext* GetContext() const;
		MeshRegistry* GetMeshRegistry() const;
		PSORegistry* GetPSORegistry() const;
		MaterialRegistry* GetMaterialRegistry() const;
		TextureRegistry* GetTextureRegistry() const;
		SamplerRegistry * GetSamplerRegistry() const;

		//View
		void CreateView(uint8_t viewId)override;
		void SetViewRTV(uint8_t viewId, ID3D11RenderTargetView* rtv)override;
		void SetViewDSV(uint8_t viewId, ID3D11DepthStencilView* dsv)override;
		void SetViewClear(uint8_t viewId, uint32_t flags, uint32_t clearColor, float depth = 1.0f, uint8_t stencil = 0)override;
		void SetViewRect(uint8_t viewId, float x, float y, float width, float height)override;
		void SetViewProj(const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj)override;

		void Resize(uint32_t width, uint32_t height) override;

		//TransientBuffer
		uint32_t AllocTransientVertexBuffer(uint32_t sizeBytes, void*& dataPtr);

		uint32_t AllocTransientIndexBuffer(uint32_t sizeBytes, void*& dataPtr);

	private:

		Microsoft::WRL::ComPtr<ID3D11Device> m_device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
		Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
		Microsoft::WRL::ComPtr<ID3D11Texture2D>           m_depthStencilBuffer;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView>    m_depthStencilView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState>   m_depthStencilState;

		HWND                m_hwnd;
		uint32_t            m_width;
		uint32_t            m_height;
		DirectX::XMMATRIX   m_view;
		DirectX::XMMATRIX   m_proj;
		DirectX::XMMATRIX   m_viewProj;

		DrawQueue                           m_drawQueue;
		std::unique_ptr<MeshRegistry>       m_meshRegistry;
		std::unique_ptr<PSORegistry>        m_psoRegistry;
		std::unique_ptr<MaterialRegistry>   m_materialRegistry;
		std::unique_ptr<TextureRegistry>   m_textureRegistry;
		std::unique_ptr<SamplerRegistry>   m_samplerRegistry;

		std::unordered_map<uint8_t, View> m_views;

	private:
		// ──────────── State Caching ────────────
		struct BoundState
		{
			ID3D11InputLayout* inputLayout = nullptr;
			D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;

			ID3D11VertexShader * vs = nullptr;
			ID3D11PixelShader* ps = nullptr;
			ID3D11GeometryShader* gs = nullptr;
			ID3D11HullShader* hs = nullptr;
			ID3D11DomainShader* ds = nullptr;

			ID3D11BlendState * blendState = nullptr;
			float                    blendFactor[4] = { 0,0,0,0 };
			UINT                     sampleMask = 0xFFFFFFFF;

			ID3D11DepthStencilState * depthStencilState = nullptr;
			UINT                     stencilRef = 0;

			ID3D11RasterizerState * rasterizerState = nullptr;
		} m_lastState;
		// ─────────────────────────────────────────

		 // Transient buffer 스트리밍용 헬퍼
		std::unique_ptr<TransientBufferAllocator> m_vbAllocator;
		std::unique_ptr<TransientBufferAllocator> m_ibAllocator;

	private:
		void bindInputLayout(ID3D11InputLayout* layout);
		void bindPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topo);
		void bindShaders(const PipelineState* pso);
		void bindBlendState(ID3D11BlendState* bs, const float bf[4], UINT mask = 0xFFFFFFFF);
		void bindDepthStencilState(ID3D11DepthStencilState* dss, UINT stencilRef);
		void bindRasterizerState(ID3D11RasterizerState* rs);
	};

	RendererAPI* CreateD3D11Renderer();
	void DestroyRenderer(RendererAPI* renderer);

};
