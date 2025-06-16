#pragma once

#include "DrawCommand.h"

#include <DirectXMath.h>
#include <d3d11.h>

namespace BinRenderer {

	enum class APIType { None, D3D11, OpenGL }; // �̷� Ȯ�� ���

	struct InitParams {
		void* windowHandle;
		int width;
		int height;
	};

	// Clear �÷��� (��Ʈ����ũ)
	enum ClearFlags : uint32_t {
		ClearColor = 1 << 0,
		ClearDepth = 1 << 1,
		ClearStencil = 1 << 2,
	};

	class RendererAPI {
	public:
		virtual ~RendererAPI() = default;

		virtual bool Init(const InitParams& params) = 0;
		virtual void BeginFrame() = 0;
		virtual void Submit() = 0; // ��: DrawCommand ť ó��
		virtual void Submit(const DrawCommand& cmd) = 0;
		virtual void EndFrame() = 0;
		virtual void Present() = 0;


		// View/FrameBuffer ����
		virtual void CreateView(uint8_t viewId) = 0;
		virtual void SetViewRTV(uint8_t viewId, ID3D11RenderTargetView* rtv) = 0;
		virtual void SetViewDSV(uint8_t viewId, ID3D11DepthStencilView* dsv) = 0;
		virtual void SetViewClear(uint8_t viewId, uint32_t flags, uint32_t clearColor, float depth = 1.0f, uint8_t stencil = 0) = 0;
		virtual void SetViewRect(uint8_t viewId, float x, float y, float width, float height) = 0;
		virtual void SetViewProj(const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj) = 0;
	};

	RendererAPI* CreateD3D11Renderer(); // ���丮
	void DestroyRenderer(RendererAPI* renderer);

}
