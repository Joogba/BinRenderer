#include "D3D11Utils.h"
#include <d3dcompiler.h>

namespace BinRenderer::D3D11Utils {

    DXGI_FORMAT ToDXGIFormat(BinRenderer::Format fmt) {
        switch (fmt)
        {
        case Format::R8G8B8A8_UNORM:      return DXGI_FORMAT_R8G8B8A8_UNORM;
        case Format::RGBA32_FLOAT:        return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case Format::R16G16B16A16_FLOAT:  return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case Format::R32_FLOAT:           return DXGI_FORMAT_R32_FLOAT;
        case Format::DEPTH24_STENCIL8:    return DXGI_FORMAT_D24_UNORM_S8_UINT;
        default:                          return DXGI_FORMAT_UNKNOWN;
        }
    }

    D3D11_PRIMITIVE_TOPOLOGY ToD3D11PrimitiveTopology(BinRenderer::PrimitiveTopology topo) {
        using PT = PrimitiveTopology;
        switch (topo) {
        case PT::TriangleList:  return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        case PT::TriangleStrip: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        case PT::LineList:      return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
        case PT::LineStrip:     return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
        case PT::PointList:     return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
        default:                return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
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

    // ------- Blend 변환 유틸 ------
    D3D11_BLEND ToD3D11Blend(BinRenderer::Blend b)
    {
        switch (b) {
        case Blend::Zero:           return D3D11_BLEND_ZERO;
        case Blend::One:            return D3D11_BLEND_ONE;
        case Blend::SrcColor:       return D3D11_BLEND_SRC_COLOR;
        case Blend::InvSrcColor:    return D3D11_BLEND_INV_SRC_COLOR;
        case Blend::SrcAlpha:       return D3D11_BLEND_SRC_ALPHA;
        case Blend::InvSrcAlpha:    return D3D11_BLEND_INV_SRC_ALPHA;
        case Blend::DestAlpha:      return D3D11_BLEND_DEST_ALPHA;
        case Blend::InvDestAlpha:   return D3D11_BLEND_INV_DEST_ALPHA;
        case Blend::DestColor:      return D3D11_BLEND_DEST_COLOR;
        case Blend::InvDestColor:   return D3D11_BLEND_INV_DEST_COLOR;
        case Blend::BlendFactor:    return D3D11_BLEND_BLEND_FACTOR;
        case Blend::InvBlendFactor: return D3D11_BLEND_INV_BLEND_FACTOR;
        default:                    return D3D11_BLEND_ONE;
        }
    }

    D3D11_BLEND_OP ToD3D11BlendOp(BinRenderer::BlendOp op)
    {
        switch (op) {
        case BlendOp::Add:         return D3D11_BLEND_OP_ADD;
        case BlendOp::Subtract:    return D3D11_BLEND_OP_SUBTRACT;
        case BlendOp::RevSubtract: return D3D11_BLEND_OP_REV_SUBTRACT;
        case BlendOp::Min:         return D3D11_BLEND_OP_MIN;
        case BlendOp::Max:         return D3D11_BLEND_OP_MAX;
        default:                   return D3D11_BLEND_OP_ADD;
        }
    }
    // ------- 실제 BlendState 변환 ------
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
            dst.SrcBlend = ToD3D11Blend(src.SrcBlend);
            dst.DestBlend = ToD3D11Blend(src.DestBlend);
            dst.BlendOp = ToD3D11BlendOp(src.Blendop);
            dst.SrcBlendAlpha = ToD3D11Blend(src.SrcBlendAlpha);
            dst.DestBlendAlpha = ToD3D11Blend(src.DestBlendAlpha);
            dst.BlendOpAlpha = ToD3D11BlendOp(src.BlendOpAlpha);
            dst.RenderTargetWriteMask = src.RenderTargetWriteMask;
        }
        return d;
    }

    // ------- DepthStencil 변환 -------
    D3D11_COMPARISON_FUNC ToD3D11ComparisonFunc(BinRenderer::ComparisonFunc f)
    {
        switch (f) {
        case ComparisonFunc::Never:        return D3D11_COMPARISON_NEVER;
        case ComparisonFunc::Less:         return D3D11_COMPARISON_LESS;
        case ComparisonFunc::Equal:        return D3D11_COMPARISON_EQUAL;
        case ComparisonFunc::LessEqual:    return D3D11_COMPARISON_LESS_EQUAL;
        case ComparisonFunc::Greater:      return D3D11_COMPARISON_GREATER;
        case ComparisonFunc::NotEqual:     return D3D11_COMPARISON_NOT_EQUAL;
        case ComparisonFunc::GreaterEqual: return D3D11_COMPARISON_GREATER_EQUAL;
        case ComparisonFunc::Always:       return D3D11_COMPARISON_ALWAYS;
        default:                           return D3D11_COMPARISON_ALWAYS;
        }
    }

    D3D11_STENCIL_OP ToD3D11StencilOp(BinRenderer::StencilOp op)
    {
        switch (op) {
        case StencilOp::Keep:    return D3D11_STENCIL_OP_KEEP;
        case StencilOp::Zero:    return D3D11_STENCIL_OP_ZERO;
        case StencilOp::Replace: return D3D11_STENCIL_OP_REPLACE;
        default:                 return D3D11_STENCIL_OP_KEEP;
        }
    }

    D3D11_DEPTH_STENCIL_DESC ToD3D11DepthStencilDesc(const BinRenderer::DepthStencilState& ds)
    {
        D3D11_DEPTH_STENCIL_DESC d = {};
        d.DepthEnable = ds.DepthEnable ? TRUE : FALSE;
        d.DepthWriteMask = ds.DepthWriteMask ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
        d.DepthFunc = ToD3D11ComparisonFunc(ds.DepthFunc);

        d.StencilEnable = ds.StencilEnable ? TRUE : FALSE;
        d.StencilReadMask = ds.StencilReadMask;
        d.StencilWriteMask = ds.StencilWriteMask;

        // Front-face ops
        d.FrontFace.StencilFunc = ToD3D11ComparisonFunc(ds.FrontFace.StencilFunc);
        d.FrontFace.StencilFailOp = ToD3D11StencilOp(ds.FrontFace.StencilFailOp);
        d.FrontFace.StencilDepthFailOp = ToD3D11StencilOp(ds.FrontFace.StencilDepthFailOp);
        d.FrontFace.StencilPassOp = ToD3D11StencilOp(ds.FrontFace.StencilPassOp);

        // Back-face ops
        d.BackFace.StencilFunc = ToD3D11ComparisonFunc(ds.BackFace.StencilFunc);
        d.BackFace.StencilFailOp = ToD3D11StencilOp(ds.BackFace.StencilFailOp);
        d.BackFace.StencilDepthFailOp = ToD3D11StencilOp(ds.BackFace.StencilDepthFailOp);
        d.BackFace.StencilPassOp = ToD3D11StencilOp(ds.BackFace.StencilPassOp);

        return d;
    }

    // ------ Rasterizer 변환 ------
    D3D11_FILL_MODE ToD3D11FillMode(BinRenderer::FillMode mode)
    {
        switch (mode) {
        case FillMode::Wireframe: return D3D11_FILL_WIREFRAME;
        case FillMode::Solid:     return D3D11_FILL_SOLID;
        default:                  return D3D11_FILL_SOLID;
        }
    }
    D3D11_CULL_MODE ToD3D11CullMode(BinRenderer::CullMode mode)
    {
        switch (mode) {
        case CullMode::None:  return D3D11_CULL_NONE;
        case CullMode::Front: return D3D11_CULL_FRONT;
        case CullMode::Back:  return D3D11_CULL_BACK;
        default:              return D3D11_CULL_BACK;
        }
    }
    D3D11_RASTERIZER_DESC ToD3D11RasterizerDesc(const BinRenderer::RasterizerState& rs)
    {
        D3D11_RASTERIZER_DESC d = {};
        d.FillMode = ToD3D11FillMode(rs.fillMode);
        d.CullMode = ToD3D11CullMode(rs.cullMode);
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