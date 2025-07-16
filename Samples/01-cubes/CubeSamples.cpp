#include "CubeSample.h"

// 큐브 정점 데이터 (플랫폼 독립, 예시)
static const BinRenderer::Vertex cubeVerts[] = {
    // ... 실제 큐브 정점 데이터(위치/노멀/UV 등) 작성 ...
};
static const uint32_t cubeIndices[] = {
    // ... 실제 큐브 인덱스 데이터 ...
};

void CubeSample::Initialize(BinRenderer::RenderManager* renderer, BinRenderer::ResourceManager* resMgr)
{
    m_renderer = renderer;
    m_resMgr = resMgr;

    // 1. 큐브 메시 등록
    BinRenderer::Mesh cubeMesh;
    cubeMesh.vertexBuffer.assign((uint8_t*)cubeVerts, (uint8_t*)(cubeVerts + sizeof(cubeVerts) / sizeof(cubeVerts[0])));
    cubeMesh.indexBuffer.assign(cubeIndices, cubeIndices + sizeof(cubeIndices) / sizeof(uint32_t));
    cubeMesh.vertexStride = sizeof(BinRenderer::Vertex);
    cubeMesh.vertexCount = sizeof(cubeVerts) / sizeof(BinRenderer::Vertex);
    cubeMesh.indexCount = sizeof(cubeIndices) / sizeof(uint32_t);
    // 기타 InputLayout 등 세팅

    m_cubeMesh = m_resMgr->Meshes().Register("cube", cubeMesh);

    // 2. 머티리얼 등록 (간단 예시)
    BinRenderer::Material cubeMat;
    // ... 머티리얼 파라미터/텍스처/PSO 핸들 등 세팅
    m_cubeMat = m_resMgr->Materials().Register("cubeMat", cubeMat);

    // 3. 변환 행렬 초기화
    m_cubeTransform = glm::mat4(1.0f);
    m_angle = 0.f;
}

void CubeSample::Update(float dt)
{
    m_angle += dt * 1.0f;
    // Y축 회전
    m_cubeTransform = glm::rotate(glm::mat4(1.0f), m_angle, glm::vec3(0, 1, 0));
}

void CubeSample::Render()
{
    // 1. DrawCommand 생성
    BinRenderer::DrawCommand cmd;
    cmd.meshHandle = m_cubeMesh;
    cmd.materialHandle = m_cubeMat;
    cmd.transform = m_cubeTransform;
    // PSO 핸들, sortKey 등 필요한 값 세팅

    m_drawQueue.Clear();
    m_drawQueue.Submit(cmd);

    // 2. Flush로 드로우 호출 (인스턴싱 등 자동 처리) 
    m_drawQueue.Flush([&](const BinRenderer::DrawCommand& dc, const std::vector<glm::mat4>& transforms, size_t count) {
        if (count > 1)
            m_renderer->DrawInstanced(dc, transforms, count);
        else
            m_renderer->DrawSingle(dc);
        });
}

void CubeSample::Shutdown()
{
    // 리소스 해제
    m_renderer->Stop();
    
}