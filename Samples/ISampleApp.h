#pragma once;
#include "Core/RenderManager.h"
#include "Resources/ResourceManager.h"

class ISampleApp {
public:
    virtual ~ISampleApp() {}
    virtual void Initialize(BinRenderer::RenderManager* renderer, BinRenderer::ResourceManager* resMgr) = 0;
    virtual void Update(float dt) = 0;
    virtual void Render() = 0;
    virtual void Shutdown() = 0;
};