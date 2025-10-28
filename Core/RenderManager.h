#pragma once
#include "DrawQueue.h"
#include "DrawCommand.h"
#include "RendererAPI.h"
#include "Resources/ResourceManager.h" // 예시
#include "StaticBatcher.h"

#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <vector>
#include <memory>
#include <unordered_map>

namespace BinRenderer {
	class RenderGraph;

    class RenderManager {
    public:
        RenderManager(RendererAPI* api, RenderGraph* renderGraph, ResourceManager* resMgr);
        ~RenderManager();

        void Submit(const DrawCommand& cmd);  // 메인(로직) 스레드
        void Start();                         // 스레드 시작
        void Stop();                          // 스레드 종료/정리

        // 명시적 동기화(옵션)
        void WaitFrame();

        void PerformStaticBatching();

    private:
        void RenderThreadFunc();

        DrawQueue m_queues[2];
        int m_writeIndex = 0, m_readIndex = 1;
        std::mutex m_queueMutex;
        std::condition_variable m_cv;
        std::atomic<bool> m_running{ false };
        std::atomic<bool> m_frameReady{ false };

        RendererAPI* m_api = nullptr;
        RenderGraph* m_renderGraph = nullptr;
        ResourceManager* m_resourceMgr = nullptr;

        std::unique_ptr<StaticBatcher> m_staticBatcher;

        std::thread m_renderThread;
        // 배칭/인스턴싱 임시 버퍼
        std::vector<DrawCommand> m_batchedCommands;
        std::vector<DrawCommand> m_instancedCommands;
    };

}