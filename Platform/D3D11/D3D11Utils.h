#pragma once
#include <d3d11.h>
#include <vector>
#include <string>
#include "Core/RenderEnums.h"
#include "Core/RenderStates.h"

namespace Binrenderer::D3D11Utils
{
	DXGI_FORMAT ToDXGIFormat(BinRenderer::Format fmt);
	D3D11_PRIMITIVE_TOPOLOGY ToD3D11PrimitiveTopology(BinRenderer::PrimitiveTopology topo);
    HRESULT CompileShaderFromFile(
        const std::wstring& filePath,
        const char* entryPoint,
        const char* target,     // e.g. "vs_5_0" or "ps_5_0"
        ID3DBlob** codeBlob);
    D3D11_BLEND_DESC ToD3D11BlendDesc(const BinRenderer::BlendState& bs);
    D3D11_DEPTH_STENCIL_DESC ToD3D11DepthStencilDesc(const BinRenderer::DepthStencilState& ds);
    D3D11_RASTERIZER_DESC ToD3D11RasterizerDesc(const BinRenderer::RasterizerState& rs);
    std::vector<D3D11_INPUT_ELEMENT_DESC> ToD3D11InputLayout(
        const std::vector<BinRenderer::InputElementDesc>& inElems)
}