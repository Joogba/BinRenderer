#include "RenderManager.h"

namespace BinRenderer {

    RenderManager::RenderManager(RendererAPI* api, RenderGraph* renderGraph, ResourceManager* resMgr)
        : m_api(api), m_renderGraph(renderGraph), m_resourceMgr(resMgr),
    {
    }

    RenderManager::~RenderManager() {
        Stop();
    }

    void RenderManager::Start() {
        m_running = true;
        m_renderThread = std::thread(&RenderManager::RenderThreadFunc, this);
        m_staticBatcher = std::make_unique<StaticBatcher>();
    }

    void RenderManager::Stop() {
        m_running = false;
        m_cv.notify_all();
        if (m_renderThread.joinable())
            m_renderThread.join();
    }

    void RenderManager::Submit(const DrawCommand& cmd) {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_queues[m_writeIndex].Submit(cmd);
    }

    void RenderManager::WaitFrame() {
        while (m_frameReady) {
            std::this_thread::yield();
        }
    }

    void RenderManager::PerformStaticBatching() {
        // m_queues[m_writeIndex]에서 정적 메시/머티리얼/텍스쳐 등 기준으로 머징
        m_batchedCommands.clear();

        m_staticBatcher->BuildBatches(m_resourceMgr->Meshes());


        for (const auto& [mat, batchMesh] : m_staticBatcher->GetBatchMeshes()) {
            DrawCommand cmd;
            cmd.meshHandle = batchMesh;
            cmd.materialHandle = mat;
            // TODO: transform, psoHandle, 필요한 정보 채우기
            m_batchedCommands.push_back(cmd);
        }
    }

    

    void RenderManager::RenderThreadFunc() {
        while (m_running) {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_cv.wait(lock, [this] { return m_frameReady || !m_running; });
            if (!m_running) break;


            // (1) 정적 배칭 → (2) 자동 인스턴싱
            PerformStaticBatching();
            PerformAutoInstancing();

            // (3) RenderGraph 실행, 인스턴싱 된 DrawCommand 전달
            if (m_renderGraph) {
                m_renderGraph->Execute(m_instancedCommands, m_api, this);
            }
            else {
                // RenderGraph 없이 직접 드로우
                for (const auto& cmd : m_instancedCommands)
                    m_api->DrawSingle(cmd);
            }


            std::swap(m_writeIndex, m_readIndex);
            m_queues[m_writeIndex].Clear();
            m_frameReady = false;
        }
    }
}
