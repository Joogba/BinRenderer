# BinRenderer (VulkanHelmet Branch)

**BinRenderer**는 Vulkan API를 기반으로 구축된 현대적인 렌더링 엔진 프로젝트입니다.  
현재 **VulkanHelmet** 브랜치는 `VulkanBistro` 브랜치를 기반으로 **코어 아키텍처 리팩토링**과 **새로운 시스템(Scene, Input, Instancing)** 도입에 초점을 맞추고 있습니다.

## 🚀 VulkanHelmet 브랜치 주요 변경 사항 (New Features)

### 1. Core Architecture Refactoring
엔진의 핵심 클래스인 `Application`과 `Renderer`를 리팩토링하여 역할과 책임을 명확히 분리했습니다.
- **Application**: 윈도우 관리, 메인 루프, 입력 위임 등을 담당하며 `VulkanResourceManager`를 통해 리소스를 관리합니다.
- **Renderer**: 렌더링 파이프라인 실행, 리소스 바인딩, 그리기 명령 기록에 집중합니다. `ResourceRegistry`를 도입하여 리소스 관리를 체계화했습니다.

### 2. Scene System
애플리케이션 로직에서 씬 데이터를 분리하기 위해 `Scene` 클래스를 도입했습니다.
- **SceneNode**: 모델, 변환(Transform), 가시성 정보를 포함하는 노드 구조체입니다.
- **Scene Class**: 씬 그래프를 관리하며, 렌더러에게 렌더링할 객체들의 정보를 제공합니다.

### 3. Input System
입력 처리를 체계화하기 위해 `InputManager`와 리스너 패턴을 적용했습니다.
- **IInputListener**: 키보드, 마우스 이벤트를 수신할 수 있는 인터페이스입니다.
- **InputManager**: GLFW 입력 이벤트를 중앙에서 처리하고 등록된 리스너들에게 전파합니다.
- **ApplicationInputHandler**: 애플리케이션 레벨의 기본 입력(카메라 이동 등)을 처리합니다.

### 4. GPU Instancing
동일한 메쉬를 여러 번 그릴 때 성능을 극대화하기 위해 **GPU Instancing**을 구현했습니다.
- **구현**: `vkCmdBindVertexBuffers`를 통해 인스턴스 버퍼를 바인딩하고, `vkCmdDrawIndexed`의 `instanceCount`를 활용하여 한 번의 드로우 콜로 여러 객체를 렌더링합니다.
- **성능**: CPU 오버헤드를 줄이고 렌더링 효율성을 크게 향상시킵니다.

---

## 🌟 기존 기능 (Inherited from VulkanBistro)

이 브랜치는 `VulkanBistro`의 모든 기능을 포함하고 있습니다:

*   **JSON 기반 Render Graph**: `RenderGraph.json`을 통한 유연한 파이프라인 구성.
*   **Deferred Rendering**: G-Buffer 기반의 지연 렌더링 및 PBR 조명 처리.
*   **Half Precision (FP16)**: 메모리 대역폭 최적화를 위한 16비트 부동소수점 포맷 사용.
*   **SSAO**: 스크린 공간 앰비언트 오클루전 지원.
*   **Tracy Profiler**: 실시간 성능 프로파일링 통합.

## 📂 프로젝트 구조 (Directory Structure)

```
BinRenderer/
├── assets/         # 셰이더, 모델 등 에셋
├── Core/           # 엔진 공통 인터페이스 및 설정
├── Vulkan/         # Vulkan 구현체 (Application, Renderer, Scene, InputManager 등)
├── RenderPass/     # Render Graph 기반 렌더 패스
├── Examples/       # 예제 및 리소스
```

## Large Assets

Due to GitHub file size limitations, the following assets must be downloaded separately:

### Bistro Model
- **Location**: `assets/models/AmazonLumberyardBistroMorganMcGuire/`
- **Download**: [Add your download link here]
- **Size**: ~288 MB
- **Source**: Amazon Lumberyard Bistro (Morgan McGuire)

### Character Animations
- **Location**: `assets/characters/Leonard/`
- **Files needed**:
  - `Bboy Hip Hop Move.fbx` (51.64 MB)
  - `Idle.fbx` (51.91 MB)
  - `Leonard.fbx` (51.43 MB)
  - `Listening To Music.fbx` (51.69 MB)
- **Download**: [Add your download link here]

After downloading, extract the files to the appropriate directories as indicated above.

## 🛠️ 빌드 및 실행 (Build & Run)

### 요구 사항 (Prerequisites)
- **Visual Studio 2022** (C++ 데스크톱 개발 워크로드)
- **Vulkan SDK** (최신 버전 권장)
- **vcpkg** (패키지 관리자)

### 의존성 라이브러리 (vcpkg)
- `glfw3`, `glm`, `assimp`, `imgui`
- `spirv-reflect`, `tracy`, `ktx`

### 빌드 방법
1. `BinRenderer.sln` 솔루션 파일을 Visual Studio 2022로 엽니다.
2. `vcpkg`가 의존성 라이브러리를 자동으로 설치합니다.
3. 솔루션을 빌드하고 실행합니다.
