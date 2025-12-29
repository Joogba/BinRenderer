# BinRenderer

**BinRenderer**는 Vulkan API를 기반으로 구축된 현대적인 렌더링 엔진 프로젝트입니다.  
현재 **RHI(Render Hardware Interface)** 추상화 계층과 **Render Graph** 시스템을 중심으로 구조를 개선하고 있으며, 유연하고 확장 가능한 렌더링 파이프라인을 제공하는 것을 목표로 합니다.

## 🌟 주요 기능 (Key Features)

### 1. RHI (Render Hardware Interface)
Vulkan API의 복잡성을 추상화하여 상위 레벨에서 효율적으로 그래픽스 리소스를 관리하고 명령을 수행할 수 있도록 설계되었습니다.
- **위치**: `RHI/`
- **특징**:
  - Vulkan 백엔드 구현 (`RHI/Vulkan/`)
  - 리소스 풀링 및 관리 (`RHIResourcePool`)
  - 커맨드 버퍼 및 동기화 객체 관리
  - 스왑체인 및 프레젠테이션 처리

### 2. Render Graph System
복잡한 렌더링 파이프라인을 그래프 형태로 구성하여 리소스 의존성을 자동으로 관리하고 최적화합니다.
- **위치**: `RenderPass/RenderGraph/`
- **특징**:
  - **Setup & Execute 분리**: 각 패스는 필요한 리소스를 선언(`Setup`)하고, 실제 렌더링 명령을 기록(`Execute`)하는 두 단계로 분리됩니다.
  - **자동 의존성 관리**: 패스 간의 리소스 읽기/쓰기 관계를 분석하여 실행 순서와 동기화(Barrier)를 자동으로 처리합니다.
  - **Transient Resource**: 프레임 내에서만 유효한 리소스의 수명 주기를 관리하여 메모리 효율성을 높입니다.

### 3. Custom Render Pass
Render Graph 시스템을 기반으로 구현된 다양한 렌더링 패스들을 제공합니다.
- **위치**: `RenderPass/`
- **구현된 패스**:
  - **Deferred Rendering**: `GBufferPassRG`, `LightingPassRG`
  - **Forward Rendering**: `ForwardPassRG`, `RHIForwardPassRG`
  - **Shadow Mapping**: `ShadowPassRG`
  - **Post Processing**: `PostProcessPassRG`
  - **GUI**: `GUIPassRG` (ImGui 통합)

## 📂 프로젝트 구조 (Directory Structure)

```
BinRenderer/
├── Core/           # 엔진 핵심 기능 (설정, 로거, 입력 등)
├── RHI/            # Render Hardware Interface (Vulkan 추상화)
│   ├── Core/       # RHI 인터페이스 및 공통 정의
│   ├── Vulkan/     # Vulkan 구체 구현
│   └── ...
├── RenderPass/     # 렌더 패스 구현 및 Render Graph 시스템
│   ├── RenderGraph/# Render Graph 코어 로직 (Builder, Graph)
│   └── *RG.cpp/h   # 개별 렌더 패스 구현체
├── Rendering/      # 렌더링 관련 고수준 클래스 (Mesh, Material, Renderer)
├── Scene/          # 씬 그래프 및 객체 관리
├── Utils/          # 유틸리티 함수
└── CMakeLists.txt  # 빌드 설정
```

## 🛠️ 빌드 및 실행 (Build & Run)

### 요구 사항 (Prerequisites)
- **Visual Studio 2022** (C++ 데스크톱 개발 워크로드)
- **Vulkan SDK** (최신 버전 권장)
- **vcpkg** (패키지 관리자)

### 의존성 라이브러리 (vcpkg)
이 프로젝트는 `vcpkg.json` 매니페스트 파일을 사용하여 의존성을 관리합니다.
- `glfw3`
- `glm`
- `assimp`
- `imgui`
- `spirv-reflect`
- `tracy`
- `ktx`

### 빌드 방법 (Visual Studio 2022)
현재 CMake 설정이 완전하지 않으므로, **Visual Studio 솔루션 파일**을 사용하여 빌드하는 것을 권장합니다.

1. **필수 설정**:
   - `vcpkg`가 설치되어 있어야 합니다.
   - `vcpkg integrate install` 명령어를 실행하여 Visual Studio 전역 통합을 활성화하는 것을 권장합니다.
   - 또는 프로젝트가 매니페스트 모드(`vcpkg.json`)를 사용하므로, Visual Studio가 이를 자동으로 감지하여 패키지를 설치합니다.

2. **프로젝트 열기**:
   - `BinRenderer.sln` 파일을 Visual Studio 2022로 엽니다.

3. **빌드**:
   - 솔루션 구성(Solution Configuration)을 `Debug` 또는 `Release`로 설정합니다.
   - 플랫폼을 `x64`로 설정합니다.
   - `솔루션 빌드 (Build Solution)`를 실행합니다. (Ctrl + Shift + B)
   - *최초 빌드 시 vcpkg가 필요한 라이브러리들을 자동으로 다운로드 및 빌드하므로 시간이 소요될 수 있습니다.*

4. **실행**:
   - `BinRenderer` 프로젝트를 시작 프로젝트로 설정(Set as Startup Project)하고 실행합니다. (F5)

## 📝 개발 현황 (Current Status)
현재 프로젝트는 **LegacyVulkan** 구조에서 **RHI 및 Render Graph** 기반의 새로운 아키텍처로 마이그레이션 및 고도화 작업이 진행 중입니다.
- `LegacyVulkan/` 폴더의 코드는 빌드에서 제외되어 있으며, 참고용으로 보존되어 있습니다.
- 주요 개발 초점은 `RenderPass/` 내의 RG(RenderGraph) 기반 패스 구현과 `RHI/` 안정화에 있습니다.
