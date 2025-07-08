#include "D3D11Utils.h"


namespace BinRenderer::D3D11Utils {

    DXGI_FORMAT ToDXGIFormat(BinRenderer::Format fmt) {
        switch (fmt) {
        case BinRenderer::Format::R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
            // ...필요한 포맷만 
        default: return DXGI_FORMAT_UNKNOWN;
        }
    }

    D3D11_PRIMITIVE_TOPOLOGY ToD3D11PrimitiveTopology(BinRenderer::PrimitiveTopology topo) {
        switch (topo) {
        case BinRenderer::PrimitiveTopology::TriangleList: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            // ...필요한 토폴로지만
        default: return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
        }
    }

    HRESULT CompileShaderFromFile(
        const std::wstring& filePath,
        const char* entryPoint,
        const char* target,     // e.g. "vs_5_0" or "ps_5_0"
        ID3DBlob** codeBlob)
    {
        UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
        flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        ID3DBlob* errorBlob = nullptr;
        HRESULT hr = D3DCompileFromFile(
            filePath.c_str(),
            nullptr,
            D3D_COMPILE_STANDARD_FILE_INCLUDE,
            entryPoint,
            target,
            flags,
            0,
            codeBlob,
            &errorBlob
        );
        if (FAILED(hr) && errorBlob) {
            OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer()));
            errorBlob->Release();
        }
        return hr;
    }

    D3D11_BLEND_DESC ToD3D11BlendDesc(const BinRenderer::BlendState& bs)
    {
        D3D11_BLEND_DESC d = {};
        d.AlphaToCoverageEnable = bs.AlphaToCoverageEnable ? TRUE : FALSE;
        d.IndependentBlendEnable = bs.IndependentBlendEnable ? TRUE : FALSE;

        for (int i = 0; i < 8; ++i)
        {
            const auto& src = bs.RenderTarget[i];
            auto& dst = d.RenderTarget[i];
            dst.BlendEnable = src.BlendEnable ? TRUE : FALSE;
            dst.SrcBlend = static_cast<D3D11_BLEND>(src.SrcBlend);
            dst.DestBlend = static_cast<D3D11_BLEND>(src.DestBlend);
            dst.BlendOp = static_cast<D3D11_BLEND_OP>(src.BlendOp);
            dst.SrcBlendAlpha = static_cast<D3D11_BLEND>(src.SrcBlendAlpha);
            dst.DestBlendAlpha = static_cast<D3D11_BLEND>(src.DestBlendAlpha);
            dst.BlendOpAlpha = static_cast<D3D11_BLEND_OP>(src.BlendOpAlpha);
            dst.RenderTargetWriteMask = src.RenderTargetWriteMask;
        }
        return d;
    }

    D3D11_DEPTH_STENCIL_DESC ToD3D11DepthStencilDesc(const BinRenderer::DepthStencilState& ds)
    {
        D3D11_DEPTH_STENCIL_DESC d = {};
        d.DepthEnable = ds.DepthEnable ? TRUE : FALSE;
        d.DepthWriteMask = ds.DepthWriteMask ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
        d.DepthFunc = static_cast<D3D11_COMPARISON_FUNC>(ds.DepthFunc);

        d.StencilEnable = ds.StencilEnable ? TRUE : FALSE;
        d.StencilReadMask = ds.StencilReadMask;
        d.StencilWriteMask = ds.StencilWriteMask;

        // Front-face ops
        d.FrontFace.StencilFunc = static_cast<D3D11_COMPARISON_FUNC>(ds.FrontFace.StencilFunc);
        d.FrontFace.StencilFailOp = static_cast<D3D11_STENCIL_OP>(ds.FrontFace.StencilFailOp);
        d.FrontFace.StencilDepthFailOp = static_cast<D3D11_STENCIL_OP>(ds.FrontFace.StencilDepthFailOp);
        d.FrontFace.StencilPassOp = static_cast<D3D11_STENCIL_OP>(ds.FrontFace.StencilPassOp);

        // Back-face ops
        d.BackFace.StencilFunc = static_cast<D3D11_COMPARISON_FUNC>(ds.BackFace.StencilFunc);
        d.BackFace.StencilFailOp = static_cast<D3D11_STENCIL_OP>(ds.BackFace.StencilFailOp);
        d.BackFace.StencilDepthFailOp = static_cast<D3D11_STENCIL_OP>(ds.BackFace.StencilDepthFailOp);
        d.BackFace.StencilPassOp = static_cast<D3D11_STENCIL_OP>(ds.BackFace.StencilPassOp);

        return d;
    }

    D3D11_RASTERIZER_DESC ToD3D11RasterizerDesc(const BinRenderer::RasterizerState& rs)
    {
        D3D11_RASTERIZER_DESC d = {};
        d.FillMode = static_cast<D3D11_FILL_MODE>(rs.fillMode);
        d.CullMode = static_cast<D3D11_CULL_MODE>(rs.cullMode);
        d.FrontCounterClockwise = rs.frontCounterClockwise ? TRUE : FALSE;
        d.DepthBias = rs.depthBias;
        d.DepthBiasClamp = rs.depthBiasClamp;
        d.SlopeScaledDepthBias = rs.slopeScaledDepthBias;
        d.DepthClipEnable = rs.depthClipEnable ? TRUE : FALSE;
        d.ScissorEnable = rs.scissorEnable ? TRUE : FALSE;
        d.MultisampleEnable = rs.multisampleEnable ? TRUE : FALSE;
        return d;
    }

    std::vector<D3D11_INPUT_ELEMENT_DESC> ToD3D11InputLayout(
        const std::vector<BinRenderer::InputElementDesc>& inElems)
    {
        std::vector<D3D11_INPUT_ELEMENT_DESC> out;
        out.reserve(inElems.size());
        for (auto& e : inElems)
        {
            D3D11_INPUT_ELEMENT_DESC d = {};
            d.SemanticName = e.SemanticName;
            d.SemanticIndex = e.SemanticIndex;
            d.Format = static_cast<DXGI_FORMAT>(e.Format);
            d.InputSlot = e.InputSlot;
            d.AlignedByteOffset = e.AlignedByteOffset;
            d.InputSlotClass = (e.InputSlotClass == 0)
                ? D3D11_INPUT_PER_VERTEX_DATA
                : D3D11_INPUT_PER_INSTANCE_DATA;
            d.InstanceDataStepRate = e.InstanceDataStepRate;
            out.push_back(d);
        }
        return out;
    }

}