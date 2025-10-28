#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "RendererAPI.h"

namespace BinRenderer {

    // 그래프 빌드 단계에서 각 패스가 사용할 텍스처/뎁스 선언
    class RenderGraphBuilder {
    public:
        RenderGraphBuilder(uint32_t w, uint32_t h)
            : m_width(w), m_height(h) {
        }

        uint32_t GetWidth() const { return m_width; }
        uint32_t GetHeight() const { return m_height; }

        // RenderTarget 선언 (MRT용)
        void DeclareRenderTarget(const char* name, const TextureDesc& desc) {
            m_textures[name] = desc;
        }
        // DepthStencil 선언
        void DeclareDepthStencil(const char* name, const TextureDesc& desc) {
            m_textures[name] = desc;
        }
        // 패스 내에서 읽기용 의존성 등록 (SRV)
        void ReadTexture(const char* name) {
            m_reads.insert(name);
        }

        void ImportBackbuffer(const char* name) {
                        // 백버퍼는 m_width×m_height, RGBA8_UNORM, RT로 바인딩된다고 가정
            TextureDesc desc;
            desc.width = m_width;
            desc.height = m_height;
            desc.format = Format::R8G8B8A8_UNORM;
            desc.bindFlags = uint32_t(BindFlags::Bind_RenderTarget) | uint32_t(BindFlags::Bind_ShaderResource);
            m_textures[name] = desc;
            m_imports.insert(name);
            
        }

        // 내부용: 선언된 TextureDesc 조회
        const TextureDesc& GetTextureDesc(const std::string& name) const {
            return m_textures.at(name);
        }
        // 그래프 실행 전, 빌더가 수집한 정보
        const std::unordered_map<std::string, TextureDesc>& GetDeclaredTextures() const {
            return m_textures;
        }
        const std::unordered_set<std::string>& GetReadDependencies() const {
            return m_reads;
        }
        const std::unordered_set<std::string>& GetImportedTextures() const {
            return m_imports;
        }

    private:
        uint32_t m_width;
        uint32_t m_height;
        std::unordered_map<std::string, TextureDesc>    m_textures;
        std::unordered_set<std::string>                 m_reads;
        std::unordered_set<std::string>                 m_imports; // 외부 import 텍스쳐
    };

} // namespace BinRenderer
