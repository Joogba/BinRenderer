#pragma once
#include "Samples/ISampleApp.h"
#include "Resources/MeshRegistry.h"
#include "Resources/MaterialRegistry.h"
#include "Core/DrawQueue.h"
#include "Core/RenderManager.h"
#include <glm/glm.hpp>

class CubeSample : public ISampleApp
{
public:
    void Initialize(BinRenderer::RenderManager* renderer, BinRenderer::ResourceManager* resMgr) override;
    void Update(float dt) override;
    void Render() override;
    void Shutdown() override;

private:
    BinRenderer::MeshHandle m_cubeMesh;
    BinRenderer::MaterialHandle m_cubeMat;
    glm::mat4 m_cubeTransform = glm::mat4(1.0f);
    float m_angle = 0.f;

    BinRenderer::RenderManager* m_renderer = nullptr;
    BinRenderer::ResourceManager* m_resMgr = nullptr;
    BinRenderer::DrawQueue m_drawQueue;
};
