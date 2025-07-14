#include <windows.h>
#include <vector>
#include <DirectXMath.h>
#include "MeshFactory.h"
#include "D3D11RendererAPI.h"
#include "DeferredRenderer.h"
#include "Vertex.h"

using namespace BinRenderer;
using namespace DirectX;

// 간단한 FPS 카메라 컨트롤
struct Camera {
    XMVECTOR eye = XMVectorSet(0, 3, -10, 0);
    XMVECTOR target = XMVectorZero();
    XMVECTOR up = XMVectorSet(0, 1, 0, 0);
    float fov = XM_PIDIV4;
    float aspect = 1.0f;
    float zn = 0.1f, zf = 1000.f;

    XMMATRIX GetView() const { return XMMatrixLookAtLH(eye, target, up); }
    XMMATRIX GetProj() const { return XMMatrixPerspectiveFovLH(fov, aspect, zn, zf); }
};

// 라이트 정보
struct Light {
    XMFLOAT3 pos;
    XMFLOAT3 color;
};

// 전역
HWND            g_hWnd = nullptr;
UINT            g_width = 1280;
UINT            g_height = 720;
DeferredRenderer g_renderer;
Camera           g_camera;
std::vector<Light>   g_lights;
std::vector<std::pair<MeshHandle, XMMATRIX>> g_meshes;

// 윈도우 프로시저
LRESULT CALLBACK WndProc(HWND h, UINT msg, WPARAM w, LPARAM l) {
    if (msg == WM_DESTROY) { PostQuitMessage(0); return 0; }
    return DefWindowProc(h, msg, w, l);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int) {
    // 1) 윈도우 클래스 & 생성
    WNDCLASS wc = { CS_OWNDC, WndProc,0,0, hInst, LoadIcon(nullptr,IDI_APPLICATION),
                    LoadCursor(nullptr,IDC_ARROW), HBRUSH(COLOR_WINDOW + 1),
                    nullptr, L"TestApp" };
    RegisterClass(&wc);
    g_hWnd = CreateWindow(L"TestApp", L"DeferredRenderer Test",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        g_width, g_height, nullptr, nullptr, hInst, nullptr);
    ShowWindow(g_hWnd, SW_SHOW);

    // 2) DeferredRenderer 초기화
    InitParams ip{ g_hWnd, (int)g_width, (int)g_height };
    g_renderer.Init(ip);

    // 3) 카메라 프로젝션 세팅
    g_camera.aspect = float(g_width) / float(g_height);

    // 4) 메쉬·조명 배치
    //    MeshBuilder를 통해 기본 큐브, 평면 생성
    auto* d3d = static_cast<D3D11RendererAPI*>(g_renderer.GetCore());
    Mesh cubeMesh = MeshFactory::CreateCube(d3d->GetDevice(), 1.0f);
    Mesh planeMesh = MeshFactory::CreatePlane(d3d->GetDevice(), 20.0f);
    auto cubeH = d3d->GetMeshRegistry()->Register(cubeMesh);
    auto planeH = d3d->GetMeshRegistry()->Register(planeMesh);

    // [TestApp.cpp 혹은 RenderSystem::PrepareFrame 등에서]
    staticBatcher.BuildBatches(meshRegistry);

    // 정적 배칭 메시들(머티리얼별) 반복
    for (const auto& [mat, batchMesh] : staticBatcher.GetBatchMeshes()) {
        DrawCommand cmd;
        cmd.meshHandle = batchMesh;
        cmd.materialHandle = mat;
        cmd.psoHandle = /* 해당 머티리얼에 맞는 PSO 핸들 */;
        cmd.transform = glm::mat4(1.0f); // 이미 월드 변환 bake됨!
        drawQueue.Submit(cmd);
    }

    // 평면: y=0
    g_meshes.push_back({ planeH, XMMatrixIdentity() });
    // 큐브 몇 개 배치
    for (int x = -2; x <= 2; x += 2)
        for (int z = -2; z <= 2; z += 2) {
            XMMATRIX t = XMMatrixTranslation(float(x * 2), 1.0f, float(z * 2));
            g_meshes.push_back({ cubeH, t });
        }

    // 여러 포인트 라이트
    g_lights = {
        {{+5,5,-5},{1,0,0}},
        {{-5,5,-5},{0,1,0}},
        {{+5,5,+5},{0,0,1}},
        {{-5,5,+5},{1,1,0}}
    };

    // 5) 메시 루프
    MSG msg;
    while (true) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // 간단히 카메라 고정, 라이트도 업데이트 없음

        // Submit 각 DrawCommand
        for (auto& [mh, mat] : g_meshes) {
            DrawCommand cmd;
            cmd.meshHandle = mh;
            cmd.materialHandle = MaterialHandle::Invalid; // DeferredRenderer 내부에서 GBufferPass가 PSO만 사용
            cmd.psoHandle = PSOHandle::Invalid;      // 실제 PSO는 패스가 바인딩
            cmd.transform = mat * g_camera.GetView() * g_camera.GetProj();
            g_renderer.Submit(cmd);
        }

        // 테스트용: 라이트 정보를 LightingPass에 올리기
        //   (LightingPass 내부에 uniform layout에 맞춰 SetUniform 호출)
        g_renderer.SetLights(g_lights.data(), (uint32_t)g_lights.size());

        // 렌더 프레임
        g_renderer.RenderFrame();
    }

    return 0;
}