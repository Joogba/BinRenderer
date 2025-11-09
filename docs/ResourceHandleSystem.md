# Resource Handle System - Usage Guide

## Overview

타입 안전한 Resource Handle 시스템으로 기존 문자열 기반 리소스 관리를 개선합니다.

## Key Benefits

### ? Before (문자열 기반)
```cpp
// ? 런타임 에러 위험
imageBuffers_["depthStencill"]->view(); // 오타! nullptr 크래시!

// ? 타입 체크 없음
auto tex = imageBuffers_["floatColor1"]; // 항상 Image2D*로 가정

// ? 성능 오버헤드
for (int i = 0; i < 1000000; i++) {
    auto tex = imageBuffers_["texture"]; // 매번 문자열 해시 계산
}
```

### ? After (Handle 기반)
```cpp
// ? 컴파일 타임 타입 체크
ImageHandle handle = depthStencilHandle_;
Image2D* texture = resourceRegistry_.getResourceAs<Image2D>(handle);

if (!texture) {
    // 명시적 에러 처리
    Logger::error("Failed to get texture");
return;
}

// ? IDE 자동완성
resourceHandles_.depthStencil // 자동완성!

// ? 성능 향상
for (int i = 0; i < 1000000; i++) {
    auto* tex = resourceRegistry_.getResource(depthStencilHandle_); // uint64_t 비교
}
```

## Basic Usage

### 1. Registry 생성

```cpp
#include "ResourceRegistry.h"

class Renderer {
private:
    Context& ctx_;
    ResourceRegistry resourceRegistry_;
    
public:
    Renderer(Context& ctx) 
        : ctx_(ctx)
        , resourceRegistry_(ctx) // Registry 초기화
    {}
};
```

### 2. 리소스 등록

#### Image 등록
```cpp
void Renderer::createTextures() {
    // Depth Stencil 생성
    auto depthTexture = std::make_unique<Image2D>(ctx_);
    depthTexture->createDepthBuffer(1920, 1080);
    
    // Registry에 등록
    ImageHandle handle = resourceRegistry_.registerImage(
        "depthStencil", 
        std::move(depthTexture)
    );
    
    if (!handle) {
  Logger::error("Failed to create depth stencil");
        throw std::runtime_error("Texture creation failed");
    }
    
    // Handle 저장
    depthStencilHandle_ = handle;
}
```

#### Buffer 등록
```cpp
void Renderer::createUniformBuffers() {
    for (uint32_t i = 0; i < kMaxFramesInFlight; ++i) {
        auto sceneBuffer = std::make_unique<MappedBuffer>(ctx_);
        sceneBuffer->createUniformBuffer(sceneUBO_);
    
        BufferHandle handle = resourceRegistry_.registerBuffer(
   std::format("sceneData_{}", i),
 std::move(sceneBuffer)
        );
        
        sceneDataHandles_.push_back(handle);
    }
}
```

### 3. 리소스 사용

#### 타입 안전 접근
```cpp
void Renderer::draw(VkCommandBuffer cmd, uint32_t frameIndex) {
    // Image2D로 타입 캐스팅
    Image2D* depthStencil = resourceRegistry_.getResourceAs<Image2D>(
      depthStencilHandle_
    );
    
    if (!depthStencil) {
 Logger::error("Invalid depth stencil handle");
  return;
    }
    
    // 안전하게 사용
    depthStencil->transitionToDepthStencilAttachment(cmd);
    VkImageView depthView = depthStencil->view();
}
```

#### 버퍼 업데이트
```cpp
void Renderer::update(uint32_t frameIndex) {
    MappedBuffer* sceneBuffer = resourceRegistry_.getResourceAs<MappedBuffer>(
        sceneDataHandles_[frameIndex]
    );
    
    if (sceneBuffer) {
        sceneBuffer->updateFromCpuData();
    } else {
  Logger::error("Failed to get scene buffer for frame {}", frameIndex);
    }
}
```

### 4. 이름으로 검색 (하위 호환성)

```cpp
// 기존 코드와의 호환성을 위해
ImageHandle handle = resourceRegistry_.findImage("depthStencil");

if (handle.isValid()) {
    Image2D* tex = resourceRegistry_.getResourceAs<Image2D>(handle);
    // ...
}
```

## Advanced Usage

### Handle 구조체로 정리

```cpp
class Renderer {
private:
    ResourceRegistry resourceRegistry_;
    
    // Named handles for quick access
    struct ResourceHandles {
  // Textures
        ImageHandle depthStencil;
        ImageHandle floatColor1;
        ImageHandle floatColor2;
        ImageHandle shadowMap;
        
        // G-Buffer
     ImageHandle gAlbedo;
        ImageHandle gNormal;
        ImageHandle gPosition;
        ImageHandle gMaterial;
        
        // Per-frame buffers
 std::vector<BufferHandle> sceneData;    // [kMaxFramesInFlight]
      std::vector<BufferHandle> skyOptions;   // [kMaxFramesInFlight]
        std::vector<BufferHandle> options;      // [kMaxFramesInFlight]
    };
    
    ResourceHandles resourceHandles_;
};
```

### 벡터로 Per-Frame 리소스 관리

```cpp
void Renderer::createPerFrameBuffers() {
    resourceHandles_.sceneData.resize(kMaxFramesInFlight);
    resourceHandles_.skyOptions.resize(kMaxFramesInFlight);
    
    for (uint32_t i = 0; i < kMaxFramesInFlight; ++i) {
        // Scene data buffer
     auto sceneBuffer = std::make_unique<MappedBuffer>(ctx_);
        sceneBuffer->createUniformBuffer(sceneUBO_);
     resourceHandles_.sceneData[i] = resourceRegistry_.registerBuffer(
     std::format("sceneData_{}", i), 
            std::move(sceneBuffer)
        );
        
        // Sky options buffer
      auto skyBuffer = std::make_unique<MappedBuffer>(ctx_);
    skyBuffer->createUniformBuffer(skyOptionsUBO_);
        resourceHandles_.skyOptions[i] = resourceRegistry_.registerBuffer(
  std::format("skyOptions_{}", i),
   std::move(skyBuffer)
      );
    }
}
```

### 리소스 통계

```cpp
void Renderer::printStats() {
    auto stats = resourceRegistry_.getStats();
    
    Logger::info("Resource Registry Statistics:");
    Logger::info("  Total Resources: {}", stats.totalResources);
    Logger::info("  Image Resources: {}", stats.imageResources);
    Logger::info("  Buffer Resources: {}", stats.bufferResources);
  Logger::info("  Named Resources: {}", stats.namedResources);
}
```

### 디버깅

```cpp
void Renderer::debugPrintAllResources() {
    // 모든 이미지 핸들 가져오기
    auto imageHandles = resourceRegistry_.getAllImageHandles();
    
Logger::info("=== Image Resources ===");
    for (const auto& handle : imageHandles) {
    auto name = resourceRegistry_.getDebugName(handle);
        Logger::info("  Handle: {} - Name: {}", 
         handle.value, 
       name.value_or("<unnamed>"));
  }
    
  // 모든 버퍼 핸들 가져오기
    auto bufferHandles = resourceRegistry_.getAllBufferHandles();
    
    Logger::info("=== Buffer Resources ===");
    for (const auto& handle : bufferHandles) {
        auto name = resourceRegistry_.getDebugName(handle);
        Logger::info("  Handle: {} - Name: {}", 
    handle.value, 
 name.value_or("<unnamed>"));
    }
}
```

## Migration Guide

### Step 1: Registry 추가

```cpp
class Renderer {
private:
    // ? 제거할 것
    // std::unordered_map<std::string, std::unique_ptr<Image2D>> imageBuffers_;

    // ? 추가
    ResourceRegistry resourceRegistry_;
  ResourceHandles resourceHandles_;
};
```

### Step 2: 리소스 생성 변경

```cpp
// ? Before
imageBuffers_["depthStencil"] = std::make_unique<Image2D>(ctx_);
imageBuffers_["depthStencil"]->createDepthBuffer(width, height);

// ? After
auto depthTexture = std::make_unique<Image2D>(ctx_);
depthTexture->createDepthBuffer(width, height);
resourceHandles_.depthStencil = resourceRegistry_.registerImage(
    "depthStencil", 
    std::move(depthTexture)
);
```

### Step 3: 리소스 사용 변경

```cpp
// ? Before
auto colorTarget = imageBuffers_["floatColor1"]->view();

// ? After
Image2D* floatColor1 = resourceRegistry_.getResourceAs<Image2D>(
    resourceHandles_.floatColor1
);

if (floatColor1) {
    auto colorTarget = floatColor1->view();
}
```

## Error Handling

```cpp
// Handle validation
if (!handle.isValid()) {
    Logger::error("Invalid handle");
    return;
}

// Resource retrieval with nullptr check
Image2D* texture = resourceRegistry_.getResourceAs<Image2D>(handle);
if (!texture) {
    Logger::error("Failed to get texture or wrong type");
    return;
}

// Name-based lookup with fallback
ImageHandle handle = resourceRegistry_.findImage("myTexture");
if (!handle) {
    Logger::warn("Texture not found, using default");
    handle = defaultTextureHandle_;
}
```

## Performance Considerations

### ? Fast Operations (O(1))
- Handle validation: `handle.isValid()`
- Resource lookup: `getResource(handle)`
- Handle comparison: `handle1 == handle2`

### ?? Slower Operations (O(log n) or O(n))
- Name-based lookup: `findImage("name")` - Use sparingly!
- Registry clear: `clear()` - Only at shutdown

### ?? Best Practices
1. **Store handles as members** - Avoid repeated name lookups
2. **Use handles for hot paths** - Reserve names for initialization
3. **Batch resource creation** - Create all resources during init
4. **Minimize registry queries** - Cache frequently used pointers

## Thread Safety

ResourceRegistry는 내부적으로 `std::shared_mutex`를 사용하여 thread-safe합니다:

- **읽기 작업** (getResource, findImage 등): 동시에 여러 스레드에서 안전
- **쓰기 작업** (registerResource, destroyResource 등): 배타적 잠금

```cpp
// Thread-safe usage
std::thread t1([&]() {
    auto* tex = resourceRegistry_.getResource(handle); // OK - Read lock
});

std::thread t2([&]() {
    auto* tex = resourceRegistry_.getResource(handle); // OK - Read lock
});
```

## Next Steps

이제 Resource Handle System이 완성되었습니다! 다음 개선 사항:

1. ? **ResourceRegistry** (완료)
2. ?? **RenderGraph Enum 전환** - 문자열 비교 제거
3. ?? **Descriptor Set 자동화** - ShaderManager 리플렉션 활용
4. ?? **Error Handling (Result<T>)** - std::expected 기반

---

**Questions?**
- GitHub Issues: [Report a bug](https://github.com/Joogba/BinRenderer/issues)
- Documentation: See `ResourceRegistry.h` for full API
