// PSORegistry.h
#pragma once
#include <unordered_map>
#include <string>
#include "Core/Handle.h"
#include "Core/RenderEnums.h"
#include "Core/RenderStates.h"

namespace BinRenderer {

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
