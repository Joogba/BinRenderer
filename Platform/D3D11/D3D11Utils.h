#pragma once
#include <d3d11.h>
#include <vector>
#include <string>
#include "Core/RenderEnums.h"
#include "Core/RenderStates.h"

namespace BinRenderer::D3D11Utils
{
    // Format 변환
    DXGI_FORMAT ToDXGIFormat(BinRenderer::Format fmt);

    // PrimitiveTopology 변환
    D3D11_PRIMITIVE_TOPOLOGY ToD3D11PrimitiveTopology(BinRenderer::PrimitiveTopology topo);

    // 셰이더 컴파일 유틸
    HRESULT CompileShaderFromFile(
        const std::wstring& filePath,
        const char* entryPoint,
        const char* target,     // e.g. "vs_5_0" or "ps_5_0"
        ID3DBlob** codeBlob);

    // BlendState 변환
    D3D11_BLEND_DESC ToD3D11BlendDesc(const BinRenderer::BlendState& bs);

    // DepthStencilState 변환
    D3D11_DEPTH_STENCIL_DESC ToD3D11DepthStencilDesc(const BinRenderer::DepthStencilState& ds);

    // RasterizerState 변환
    D3D11_RASTERIZER_DESC ToD3D11RasterizerDesc(const BinRenderer::RasterizerState& rs);

    // InputElementDesc 변환
    std::vector<D3D11_INPUT_ELEMENT_DESC> ToD3D11InputLayout(
        const std::vector<BinRenderer::InputElementDesc>& inElems);

    // Blend 변환
    D3D11_BLEND ToD3D11Blend(BinRenderer::Blend blend);

    // BlendOp 변환
    D3D11_BLEND_OP ToD3D11BlendOp(BinRenderer::BlendOp op);

    // ComparisonFunc 변환
    D3D11_COMPARISON_FUNC ToD3D11ComparisonFunc(BinRenderer::ComparisonFunc f);

    // FillMode 변환
    D3D11_FILL_MODE ToD3D11FillMode(BinRenderer::FillMode mode);

    // CullMode 변환
    D3D11_CULL_MODE ToD3D11CullMode(BinRenderer::CullMode mode);

    // AddressMode 변환 (샘플러 등에서)
    D3D11_TEXTURE_ADDRESS_MODE ToD3D11AddressMode(BinRenderer::AddressMode mode);

    // FilterMode 변환
    D3D11_FILTER ToD3D11Filter(BinRenderer::FilterMode filter, bool comparison = false);
}