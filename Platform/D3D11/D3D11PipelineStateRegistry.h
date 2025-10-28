#pragma once
#include <d3d11.h>
#include <unordered_map>
#include <memory>

#include "Resources/PSORegistry.h"
#include "D3D11Utils.h"

namespace BinRenderer
{

    struct D3D11PipelineState {
        ID3D11InputLayout* inputLayout = nullptr;
        ID3D11VertexShader* vertexShader = nullptr;
        ID3D11PixelShader* pixelShader = nullptr;
        ID3D11HullShader* hullShader = nullptr;
        ID3D11DomainShader* domainShader = nullptr;
        ID3D11GeometryShader* geometryShader = nullptr;
        ID3D11BlendState* blendState = nullptr;
        float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        UINT sampleMask = 0xFFFFFFFF;
        ID3D11DepthStencilState* depthStencilState = nullptr;
        UINT stencilRef = 0;
        ID3D11RasterizerState* rasterizerState = nullptr;
        D3D11_PRIMITIVE_TOPOLOGY primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
		// 기타 필요한 상태들
    };

    std::unique_ptr<D3D11PipelineState> CreatePipelineState(
        ID3D11Device* device,
        const PSODesc& desc,
        const std::unordered_map<ShaderHandle, ShaderBlob>& shaderBlobs)
    {
        auto pso = std::make_unique<D3D11PipelineState>();

        // 1. 입력 레이아웃 변환 및 생성
        std::vector<D3D11_INPUT_ELEMENT_DESC> inputDescs;
        for (const auto& elem : desc.inputLayout) {
            D3D11_INPUT_ELEMENT_DESC d{};
            d.SemanticName = elem.SemanticName;
            d.SemanticIndex = elem.SemanticIndex;
            d.Format = DXGIFormatFromFormat(elem.Format); // 변환 함수 필요
            d.InputSlot = elem.InputSlot;
            d.AlignedByteOffset = elem.AlignedByteOffset;
            d.InputSlotClass = (elem.InputSlotClass == 0) ? D3D11_INPUT_PER_VERTEX_DATA : D3D11_INPUT_PER_INSTANCE_DATA;
            d.InstanceDataStepRate = elem.InstanceDataStepRate;
            inputDescs.push_back(d);
        }
        // 셰이더 바이트코드 필요 (바이트코드 관리 필요)
        auto vsBlob = shaderBlobs.at(desc.vertexShader);
        HRESULT hr = device->CreateInputLayout(
            inputDescs.data(), (UINT)inputDescs.size(),
            vsBlob.data(), vsBlob.size(),
            &pso->inputLayout);
        assert(SUCCEEDED(hr) && "CreateInputLayout failed!");

        // 2. 셰이더 객체 생성
        // 이미 shaderBlobs에 컴파일된 바이트코드가 있다고 가정
        hr = device->CreateVertexShader(vsBlob.data(), vsBlob.size(), nullptr, &pso->vertexShader);
        assert(SUCCEEDED(hr));
        if (desc.pixelShader.isValid()) {
            auto psBlob = shaderBlobs.at(desc.pixelShader);
            hr = device->CreatePixelShader(psBlob.data(), psBlob.size(), nullptr, &pso->pixelShader);
            assert(SUCCEEDED(hr));
        }
        // 기타 hull/domain/geometry 셰이더도 유사

        // 3. BlendState 생성
        D3D11_BLEND_DESC blendDesc = ToD3D11BlendDesc(desc.blendState); // 변환 함수 필요
        hr = device->CreateBlendState(&blendDesc, &pso->blendState);
        assert(SUCCEEDED(hr));

        // 4. DepthStencilState 생성
        D3D11_DEPTH_STENCIL_DESC depthDesc = ToD3D11DepthStencilDesc(desc.depthStencilState);
        hr = device->CreateDepthStencilState(&depthDesc, &pso->depthStencilState);
        assert(SUCCEEDED(hr));

        // 5. RasterizerState 생성
        D3D11_RASTERIZER_DESC rastDesc = ToD3D11RasterizerDesc(desc.rasterizerState);
        hr = device->CreateRasterizerState(&rastDesc, &pso->rasterizerState);
        assert(SUCCEEDED(hr));

        // 6. 기타 정보 복사
        pso->primitiveTopology = ToD3D11Topology(desc.primitiveTopology);
        for (int i = 0; i < 4; ++i) pso->blendFactor[i] = desc.blendFactor[i];
        pso->stencilRef = desc.stencilRef;
        pso->sampleMask = desc.sampleMask;

        // RTV/DSV 포맷 등도 필요하면 DXGI_FORMAT으로 변환

        return pso;
    }

    class D3D11PSORegistry {
    public:
        D3D11PipelineState* GetOrCreate(const PSODesc& desc) {
            auto it = m_cache.find(desc);
            if (it != m_cache.end())
                return it->second.get();
            // 변환 & 생성
            auto pso = std::make_unique<D3D11PipelineState>();
            // 1. desc.inputLayout → D3D11_INPUT_ELEMENT_DESC[]
            // 2. device->CreateInputLayout(..., &pso->inputLayout)
            // 3. desc.shader 코드 → device->CreateVertexShader/PixelShader...
            // ... 등등
            // 4. 캐싱
            auto* result = pso.get();
            m_cache[desc] = std::move(pso);
            return result;
        }
    private:
        std::unordered_map<PSODesc, std::unique_ptr<D3D11PipelineState>, PSODescHash> m_cache;
    };
}