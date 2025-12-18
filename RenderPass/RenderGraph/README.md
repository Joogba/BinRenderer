# RenderGraph System

## ?? 개요

**RenderGraph**는 현대적인 렌더링 엔진을 위한 선언적(declarative) 렌더링 파이프라인 시스템입니다.

### 주요 특징

? **자동 의존성 관리**: Read/Write 선언만으로 실행 순서 자동 결정  
? **리소스 생명주기 추적**: 자동 할당/해제로 메모리 누수 방지  
? **타입 안전성**: 템플릿 기반 핸들로 컴파일 타임 검증  
? **확장 가능한 구조**: 새로운 패스 추가가 간편  
? **플랫폼 독립적**: RHI 추상화 계층 사용  

---

## ?? 빠른 시작

### 1. 기본 구조

```cpp
#include "RenderPass/RenderGraph/RGGraph.h"

// RenderGraph 생성
RenderGraph renderGraph(rhi);

// 패스 추가
struct MyPassData {
    RGTextureHandle output;
};

renderGraph.addPass<MyPassData>("MyPass",
    // Setup: 리소스 선언
    [](MyPassData& data, RenderGraphBuilder& builder) {
        RGTextureDesc desc;
   desc.name = "Output";
        desc.width = 1920;
        desc.height = 1080;
   desc.format = RHI_FORMAT_R8G8B8A8_UNORM;
    desc.usage = RHI_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        data.output = builder.writeTexture(builder.createTexture(desc));
        builder.setFinalOutput(data.output);
    },
    // Execute: 실제 렌더링
    [](const MyPassData& data, RHI* rhi, uint32_t frameIndex) {
        // 렌더링 커맨드 기록
    }
);

// 컴파일 및 실행
renderGraph.compile();
renderGraph.execute(frameIndex);
```

### 2. Ex01_Context 프로젝트에서 사용

```cpp
// main.cpp
#include "../Examples/RenderGraph_Example.cpp"

int main()
{
    // RHI 초기화
    RHI* rhi = ...;
    
    // 예제 실행
    BinRenderer::RunSimpleRenderGraphExample(rhi);
    
    return 0;
}
```

---

## ?? 파일 구조

```
RenderPass/RenderGraph/
├── RGTypes.h       # 핸들 및 리소스 설명자
├── RGPass.h    # 패스 추상화
├── RGBuilder.h/cpp    # 리소스 빌더
├── RGGraph.h/cpp  # 메인 그래프
└── README.md     # 이 파일

Examples/
└── RenderGraph_Example.cpp  # 간단한 예제

Docs/
├── RenderGraph_Guide.md     # 상세 가이드
└── RenderGraph_Implementation_Summary.md  # 구현 요약
```

---

## ?? 주요 개념

### 리소스 핸들
```cpp
RGTextureHandle texture = builder.createTexture(desc);
RGBufferHandle buffer = builder.createBuffer(desc);
```

### 의존성 선언
```cpp
builder.readTexture(handle);   // 읽기 (의존성)
builder.writeTexture(handle);  // 쓰기 (생성)
builder.readWriteTexture(handle); // 읽기+쓰기
```

### 자동 실행 순서
```
PassA writes T1 → PassB reads T1
→ RenderGraph ensures: PassA → PassB
```

---

## ?? 참고 문서

- **상세 가이드**: `../../Docs/RenderGraph_Guide.md`
- **구현 요약**: `../../Docs/RenderGraph_Implementation_Summary.md`
- **예제 코드**: `../../Examples/RenderGraph_Example.cpp`

---

## ??? 로드맵

- [x] 기본 패스 시스템
- [x] 자동 의존성 해결
- [x] 리소스 할당/해제
- [ ] Resource Aliasing
- [ ] Async Compute
- [ ] Graphviz 시각화

---

**구현 완료!** ??
