#include "D3D11RendererAPI.h"
#include "DrawCommand.h"
#include "MeshRegistry.h"
#include "PSORegistry.h"
#include "MaterialSystem.h"

#include <d3d11.h>
#include <memory>

namespace BinRenderer
{
    D3D11RendererAPI::D3D11RendererAPI() : m_hwnd(nullptr) {}

    D3D11RendererAPI::~D3D11RendererAPI() {}

    bool D3D11RendererAPI::Init(const InitParams& params) {
        m_hwnd = static_cast<HWND>(params.windowHandle);

        DXGI_SWAP_CHAIN_DESC scDesc = {};
        scDesc.BufferCount = 1;
        scDesc.BufferDesc.Width = params.width;
        scDesc.BufferDesc.Height = params.height;
        scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scDesc.OutputWindow = m_hwnd;
        scDesc.SampleDesc.Count = 1;
        scDesc.Windowed = TRUE;

        D3D_FEATURE_LEVEL featureLevel;
        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
            nullptr, 0,
            D3D11_SDK_VERSION,
            &scDesc,
            &m_swapChain,
            &m_device,
            &featureLevel,
            &m_context
        );

        if (FAILED(hr)) return false;

        Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
        m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
        m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_renderTargetView);

        m_meshRegistry = std::make_unique<MeshRegistry>();
        m_psoRegistry = std::make_unique<PSORegistry>();
        m_materialRegistry = std::make_unique<MaterialRegistry>();

        return true;
    }

    void D3D11RendererAPI::BeginFrame() {
        float clearColor[4] = { 0.1f, 0.1f, 0.3f, 1.0f };
        m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);
        m_context->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
    }

    void D3D11RendererAPI::Submit(const DrawCommand& cmd) {
        // DrawCommands 처리 예정
        m_drawQueue.Submit(cmd);
    }

    void D3D11RendererAPI::Submit()
    {
        for (const auto& cmd : m_drawQueue.GetCommands()) {
            const Mesh* mesh = m_meshRegistry->Get(cmd.meshHandle);
            const Material* material = m_materialRegistry->Get(cmd.materialHandle);
            if (!mesh || !material) continue;

            const PipelineState* pso = m_psoRegistry->Get(material->GetPSO());
            if (!pso) continue;


            m_context->IASetVertexBuffers(0,1, &mesh->vertexBuffer, &mesh->vertexStride, &mesh->vertexOffset);
            m_context->IASetIndexBuffer(mesh->indexBuffer, DXGI_FORMAT_R32_UINT, 0);
            m_context->IASetInputLayout(pso->m_inputLayout.Get());
            m_context->IASetPrimitiveTopology(pso->m_primitiveTopology);

            m_context->VSSetShader(pso->m_vertexShader.Get(), nullptr, 0);
            m_context->PSSetShader(pso->m_pixelShader.Get(), nullptr, 0);
            if (pso->m_geometryShader) m_context->GSSetShader(pso->m_geometryShader.Get(), nullptr, 0);
            if (pso->m_hullShader)     m_context->HSSetShader(pso->m_hullShader.Get(), nullptr, 0);
            if (pso->m_domainShader)   m_context->DSSetShader(pso->m_domainShader.Get(), nullptr, 0);

            m_context->OMSetBlendState(pso->m_blendState.Get(), pso->m_blendFactor, 0xffffffff);
            m_context->OMSetDepthStencilState(pso->m_depthStencilState.Get(), pso->m_stencilRef);
            m_context->RSSetState(pso->m_rasterizerState.Get());

            // UniformSet으로 상수버퍼 업데이트
            const UniformSet& uniforms = material->GetUniformSet();
            D3D11_BUFFER_DESC cbDesc = {};
            cbDesc.ByteWidth = uniforms.GetSize();
            cbDesc.Usage = D3D11_USAGE_DEFAULT;
            cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

            D3D11_SUBRESOURCE_DATA initData = {};
            initData.pSysMem = uniforms.GetRawData();

            Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;
            HRESULT hr = m_device->CreateBuffer(&cbDesc, &initData, &constantBuffer);
            if (SUCCEEDED(hr)) {
                m_context->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());
                m_context->PSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());
            }

            m_context->DrawIndexed(mesh->indexCount, 0, 0);

        }
        m_drawQueue.Clear();
    }


    void D3D11RendererAPI::EndFrame() {
        // Post-processing 등 예정
    }

    void D3D11RendererAPI::Present() {
        m_swapChain->Present(1, 0);
    }

    RendererAPI* CreateD3D11Renderer() {
        return new D3D11RendererAPI();
    }

    void DestroyRenderer(RendererAPI* renderer) {
        delete renderer;
    }

}