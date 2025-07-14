// PSORegistry.h
#pragma once
#include <unordered_map>
#include <string>
#include <functional>

#include "Core/Handle.h"
#include "Core/RenderEnums.h"
#include "Core/RenderStates.h"

namespace BinRenderer {

    // 해시 결합 유틸리티
    inline void HashCombine(size_t& seed, size_t value) {
        seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    inline bool operator==(const PSODesc& a, const PSODesc& b) {
        return a.vertexShader == b.vertexShader &&
            a.pixelShader == b.pixelShader &&
            a.hullShader == b.hullShader &&
            a.domainShader == b.domainShader &&
            a.geometryShader == b.geometryShader &&
            a.inputLayout == b.inputLayout &&
            a.blendState == b.blendState &&
            a.depthStencilState == b.depthStencilState &&
            a.rasterizerState == b.rasterizerState &&
            a.blendFactor == b.blendFactor &&
            a.stencilRef == b.stencilRef &&
            a.primitiveTopology == b.primitiveTopology &&
            a.sampleMask == b.sampleMask &&
            a.rtvFormats == b.rtvFormats &&
            a.dsvFormat == b.dsvFormat &&
            a.numRenderTargets == b.numRenderTargets;
    }

    struct PSODesc 
    { 
        // 셰이더 핸들(플랫폼 독립!)
        ShaderHandle vertexShader;
        ShaderHandle pixelShader;
        ShaderHandle hullShader;
        ShaderHandle domainShader;
        ShaderHandle geometryShader;

        // 입력 레이아웃(플랫폼 독립 구조체)
        std::vector<InputElementDesc> inputLayout;

        // 상태 객체(플랫폼 독립 구조체)
        BlendState         blendState;
        DepthStencilState  depthStencilState;
        RasterizerState    rasterizerState;

        // 파라미터(플랫폼 독립)
        std::array<float, 4> blendFactor = { 1.0f, 1.0f, 1.0f, 1.0f };
        uint32_t            stencilRef = 0;
        PrimitiveTopology   primitiveTopology = PrimitiveTopology::TriangleList;
        uint32_t            sampleMask = 0xFFFFFFFF;

        // 필요한 경우: color/depth 스펙, 멀티렌더타겟 등
        std::array<Format, 8> rtvFormats = { Format::Unknown };
        Format               dsvFormat = Format::Unknown;
        uint32_t             numRenderTargets = 1;
        // 기타 확장 필요시 필드 추가!
    };

    struct PSODescHash {
        size_t operator()(const PSODesc& desc) const {
            size_t h = 0;
            // 셰이더 핸들 해시
            HashCombine(h, std::hash<uint32_t>()(static_cast<uint32_t>(desc.vertexShader)));
            HashCombine(h, std::hash<uint32_t>()(static_cast<uint32_t>(desc.pixelShader)));
            HashCombine(h, std::hash<uint32_t>()(static_cast<uint32_t>(desc.hullShader)));
            HashCombine(h, std::hash<uint32_t>()(static_cast<uint32_t>(desc.domainShader)));
            HashCombine(h, std::hash<uint32_t>()(static_cast<uint32_t>(desc.geometryShader)));
            // 입력 레이아웃
            for (const auto& elem : desc.inputLayout) {
                size_t elemHash = std::hash<std::string>()(elem.SemanticName);
                HashCombine(elemHash, std::hash<uint32_t>()(elem.SemanticIndex));
                HashCombine(elemHash, std::hash<int>()(static_cast<int>(elem.Format)));
                HashCombine(elemHash, std::hash<uint32_t>()(elem.InputSlot));
                HashCombine(elemHash, std::hash<uint32_t>()(elem.AlignedByteOffset));
                HashCombine(elemHash, std::hash<uint32_t>()(elem.InputSlotClass));
                HashCombine(elemHash, std::hash<uint32_t>()(elem.InstanceDataStepRate));
                HashCombine(h, elemHash);
            }
            // 상태 해시
            HashCombine(h, std::hash<int>()(static_cast<int>(desc.blendState.blendOp)));
            HashCombine(h, std::hash<int>()(static_cast<int>(desc.depthStencilState.DepthFunc)));
            HashCombine(h, std::hash<int>()(static_cast<int>(desc.rasterizerState.cullMode)));

            // BlendFactor
            for (auto v : desc.blendFactor)
                HashCombine(h, std::hash<float>()(v));
            HashCombine(h, std::hash<uint32_t>()(desc.stencilRef));
            HashCombine(h, std::hash<int>()(static_cast<int>(desc.primitiveTopology)));
            HashCombine(h, std::hash<uint32_t>()(desc.sampleMask));

            // RenderTargetFormats
            for (auto f : desc.rtvFormats)
                HashCombine(h, std::hash<int>()(static_cast<int>(f)));
            HashCombine(h, std::hash<int>()(static_cast<int>(desc.dsvFormat)));
            HashCombine(h, std::hash<uint32_t>()(desc.numRenderTargets));
            return h;
        }
    };



    class PSORegistry {
    public:
        PSOHandle Register(const std::string& name, const PSODesc& pso) {
            auto it = m_nameToIdx.find(name);
            if (it != m_nameToIdx.end()) return PSOHandle(it->second);
            PSOHandle handle(m_nextId++);
            m_psos.emplace(handle.idx, pso);
            m_nameToIdx[name] = handle.idx;
            m_idxToName[handle.idx] = name;
            return handle;
        }

        const PSODesc* Get(PSOHandle handle) const {
            auto it = m_psos.find(handle.idx);
            return it != m_psos.end() ? &it->second : nullptr;
        }

        const PSODesc* Get(const std::string& name) const {
            auto it = m_nameToIdx.find(name);
            return it != m_nameToIdx.end() ? Get(PSOHandle(it->second)) : nullptr;
        }

        PSOHandle GetHandle(const std::string& name) const {
            auto it = m_nameToIdx.find(name);
            return it != m_nameToIdx.end() ? PSOHandle(it->second) : PSOHandle();
        }

        const std::string& GetName(PSOHandle handle) const {
            static std::string empty;
            auto it = m_idxToName.find(handle.idx);
            return it != m_idxToName.end() ? it->second : empty;
        }

    private:
        std::unordered_map<uint16_t, PSODesc> m_psos;
        std::unordered_map<std::string, uint16_t> m_nameToIdx;
        std::unordered_map<uint16_t, std::string> m_idxToName;
        uint16_t m_nextId = 1;
    };

} // namespace BinRenderer
