#pragma once

#include <d3d11.h>
#include <wrl/client.h>

namespace BinRenderer {

    using Microsoft::WRL::ComPtr;

    class PipelineState {
    public:
        void SetBlendFactor(const float blendFactor[4]) {
            for (int i = 0; i < 4; ++i)
                m_blendFactor[i] = blendFactor[i];
        }

        void operator=(const PipelineState& pso) {
            m_vertexShader = pso.m_vertexShader;
            m_pixelShader = pso.m_pixelShader;
            m_hullShader = pso.m_hullShader;
            m_domainShader = pso.m_domainShader;
            m_geometryShader = pso.m_geometryShader;
            m_inputLayout = pso.m_inputLayout;

            m_blendState = pso.m_blendState;
            m_depthStencilState = pso.m_depthStencilState;
            m_rasterizerState = pso.m_rasterizerState;

            for (int i = 0; i < 4; ++i)
                m_blendFactor[i] = pso.m_blendFactor[i];

            m_stencilRef = pso.m_stencilRef;
            m_primitiveTopology = pso.m_primitiveTopology;
        }

    public:
        ComPtr<ID3D11VertexShader>     m_vertexShader;
        ComPtr<ID3D11PixelShader>      m_pixelShader;
        ComPtr<ID3D11HullShader>       m_hullShader;
        ComPtr<ID3D11DomainShader>     m_domainShader;
        ComPtr<ID3D11GeometryShader>   m_geometryShader;
        ComPtr<ID3D11InputLayout>      m_inputLayout;

        ComPtr<ID3D11BlendState>       m_blendState;
        ComPtr<ID3D11DepthStencilState> m_depthStencilState;
        ComPtr<ID3D11RasterizerState>  m_rasterizerState;

        float m_blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        UINT  m_stencilRef = 0;

        D3D11_PRIMITIVE_TOPOLOGY m_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        UINT m_sampleMask = 0xFFFFFFFF;
    };

} // namespace BinRenderer
