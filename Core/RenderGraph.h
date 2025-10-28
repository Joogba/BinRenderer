#pragma once
#include "IRenderPass.h"
#include "RenderGraphBuilder.h"
#include "PassResources.h"

#include <vector>
#include <memory>

namespace BinRenderer {

    class RenderGraph {
    public:
        RenderGraph(RendererAPI* rhi, uint32_t w, uint32_t h)
            : m_rhi(rhi), m_width(w), m_height(h) {
        }

        void AddPass(std::unique_ptr<IRenderPass> pass) {
            m_passes.push_back(std::move(pass));
        }

        /// 크기 바뀌었을 때 호출
        void Resize(uint32_t w, uint32_t h) {
            if (w == m_width && h == m_height) return;
            m_width = w;
            m_height = h;
            //  이미 생성된 리소스 맵을 비워서 다음 Build()에서
            //  모두 다시 생성되도록 함
            m_globalResources.rtvs.clear();
            m_globalResources.srvs.clear();
            m_globalResources.dsvs.clear();
            //m_globalResources = PassResources();
        }

        // 1) Graph Build : Declare all passes -> create resources
        void Build() {
            RenderGraphBuilder builder(m_width, m_height);

            // Declare
            for (auto& p : m_passes) {
                p->Declare(builder);
            }

            // create texture / views
            for (auto& [name, desc] : builder.GetDeclaredTextures()) {
                auto tex = m_rhi->CreateTexture(desc);
                // RenderTarget인지 DepthStencil인지 구별해서 뷰 생성
                if (desc.bindFlags & uint32_t(BindFlags::Bind_RenderTarget)) {
                    auto rtv = m_rhi->CreateRTV(tex);
                    m_globalResources.rtvs[name] = rtv;
                }
                if (desc.bindFlags & uint32_t(BindFlags::Bind_DepthStencil)) {
                    auto dsv = m_rhi->CreateDSV(tex);
                    m_globalResources.dsvs[name] = dsv;
                }
                if (desc.bindFlags & uint32_t(BindFlags::Bind_ShaderResource)) {
                    auto srv = m_rhi->CreateSRV(tex);
                    m_globalResources.srvs[name] = srv;
                }
            }
        }

        // Execute : provide Execute, PassResources every passes in order
        void Execute() {
            for (auto& p : m_passes) {
                p->Execute(m_rhi, m_globalResources);
            }
        }


    private:
        RendererAPI* m_rhi;
        uint32_t             m_width, m_height;

        std::vector<std::unique_ptr<IRenderPass>> m_passes;
        PassResources        m_globalResources;
    };

} // namespace BinRenderer