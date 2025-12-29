# BinRenderer (VulkanBistro Branch)

**BinRenderer**는 Vulkan API를 기반으로 구축된 현대적인 렌더링 엔진 프로젝트입니다.  
현재 **VulkanBistro** 브랜치에서는 **JSON 기반 렌더 그래프**, **지연 렌더링(Deferred Rendering)**, **Half Precision(FP16)** 최적화, **Tracy 프로파일러**, **SSAO** 등이 구현되어 있습니다.

> **참고**: `main` 브랜치에서 기존 Vulkan 구현은 `LegacyVulkan/` 폴더로 이동되었습니다

## 🌟 주요 기능 (Key Features)

### 1. JSON 기반 Render Graph System
복잡한 렌더링 파이프라인을 JSON 파일로 정의하고 구성할 수 있습니다.
- **유연성**: 코드를 수정하지 않고도 렌더 패스의 연결과 구성을 변경할 수 있습니다.
- **구조**: `RenderGraph.json`을 통해 패스 간의 의존성, 입출력 리소스(Attachment)를 정의합니다.
- **예시**: `Examples/Ex01_Context/RenderGraph.json`

### 2. Deferred Rendering (지연 렌더링)
G-Buffer를 활용한 지연 렌더링 파이프라인을 지원합니다.
- **G-Buffer 구성**: Albedo, Normal, Position, Material 정보를 별도의 텍스처에 저장.
- **Lighting Pass**: G-Buffer 데이터를 기반으로 조명 연산을 수행하여 다수의 광원을 효율적으로 처리.
- **PBR**: 물리 기반 렌더링(Physically Based Rendering) 지원.

### 3. Half Precision (FP16) 최적화
메모리 대역폭 절약과 성능 향상을 위해 16비트 부동소수점(Half Precision) 형식을 적극적으로 활용합니다.
- **포맷**: `VK_FORMAT_R16G16B16A16_SFLOAT` 등을 사용하여 HDR 렌더링 및 G-Buffer 저장 시 메모리 사용량을 최적화합니다.
- **성능**: 대역폭 병목을 줄이고 캐시 효율성을 높입니다.

### 4. SSAO (Screen Space Ambient Occlusion)
스크린 공간 앰비언트 오클루전을 통해 씬의 깊이감을 더해줍니다.
- **구현**: 깊이 버퍼(Depth Buffer)를 활용하여 차폐(Occlusion)를 계산하고, 이를 조명에 반영하여 더욱 사실적인 음영을 표현합니다.
- **설정**: 반경(Radius), 바이어스(Bias), 샘플 수 등을 조절 가능.

### 5. Tracy Profiler 통합
실시간 성능 분석을 위해 **Tracy Profiler**가 통합되어 있습니다.
- **기능**: CPU 및 GPU 타임라인 프로파일링, 메모리 할당 추적, 프레임 타임 분석 등을 지원합니다.
- **사용**: `vcpkg`를 통해 `tracy` 라이브러리를 사용하며, 개발 중 성능 병목 지점을 쉽게 파악할 수 있습니다.

## 📂 프로젝트 구조 (Directory Structure)

```
BinRenderer/
├── asset/          # 셰이더, 모델 등 에셋
├── Core/           # 엔진 핵심 기능
├── RHI/            # Render Hardware Interface
├── RenderPass/     # 렌더 패스 구현 (GBuffer, Lighting, SSAO 등)
│   ├── RenderGraph/# Render Graph 시스템 및 JSON 파서
│   └── ...
├── LegacyVulkan/   # (참고용) 기존 Vulkan 구현체
├── Examples/       # 예제 및 리소스 (RenderGraph.json 등)
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
이 프로젝트는 `vcpkg.json` 매니페스트 파일을 사용하여 의존성을 관리합니다.
- `glfw3`, `glm`, `assimp`, `imgui`
- `spirv-reflect`
- `tracy` (프로파일링)
- `ktx` (텍스처)

### 빌드 방법 (Visual Studio 2022)
1. **필수 설정**:
   - `vcpkg` 설치 및 `vcpkg integrate install` 실행.
   - Visual Studio가 `vcpkg.json`을 감지하여 자동으로 라이브러리를 설치합니다.

2. **프로젝트 열기 및 빌드**:
   - `BinRenderer.sln` 파일을 엽니다.
   - 구성: `Debug` 또는 `Release`, 플랫폼: `x64`.
   - 솔루션 빌드 (Ctrl + Shift + B).

3. **실행**:
   - `Ex_Context01` 프로젝트를 시작 프로젝트로 설정하고 실행합니다 (F5).