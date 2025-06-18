// TestApp.cpp
#include <windows.h>
#include <cassert>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <memory>


#include "D3D11RendererAPI.h"
#include "GeometryGenerator.h"
#include "MeshFactory.h"
#include "DrawCommand.h"
#include "Handle.h"
#include "TextureLoader.h"
 
using namespace BinRenderer;
using namespace DirectX;

int newW = 0;
int newH = 0;

RendererAPI* renderer;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	    case WM_SIZE:
            /*if (renderer)
            {
                newW = LOWORD(lParam);
                newH = HIWORD(lParam);
                if (newW > 0 && newH > 0)
                {
                    auto renderer = reinterpret_cast<D3D11RendererAPI*>(
                        GetWindowLongPtr(hWnd, GWLP_USERDATA)
                        );
                    if (renderer)
                    {

                        renderer->Resize(newW, newH);
                    }
                }
            }*/
            break;
		    // 최소화(0×0) 땐 스킵
		    
        case WM_DESTROY:
		    PostQuitMessage(0);
		    return 0;
	}
	
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, PSTR, int nCmdShow)
{
    // 1) 윈도우 생성
    const wchar_t CLASS_NAME[] = L"BinRendererWindowClass";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClass(&wc);

    const int width = 1280;
    const int height = 720;
    DWORD style = WS_OVERLAPPEDWINDOW;
    RECT rect = { 0, 0, width, height };
    AdjustWindowRect(&rect, style, FALSE);

    HWND hWnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"BinRenderer TestApp",
        style,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        nullptr,
        nullptr,
        hInst,
        nullptr
    );
    assert(hWnd && "Failed to create window");
    renderer = CreateD3D11Renderer();
    InitParams params{ hWnd, 1280, 720 };
    assert(renderer->Init(params) && "Init failed");

    SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(renderer));
    ShowWindow(hWnd, nCmdShow);

    // 2) 렌더러 생성 & 초기화
    

    // 2.1) 카메라(뷰·투영) 행렬 설정
    using namespace DirectX;
    // 간단히 원점 쪽으로 Z축 -3 떨어진 위치에서 바라보기
    XMVECTOR eye = XMVectorSet(0.0f, 0.0f, -10.0f, 0.0f);
    XMVECTOR at = XMVectorZero();
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMMATRIX view = XMMatrixLookAtLH(eye, at, up);
    XMMATRIX proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, (float)width / (float)height, 0.1f, 100.0f);
    renderer->SetViewProj(view, proj);

    // 3) 기하 생성 (앱 레벨)
    MeshData quadData = GeometryGenerator::MakeSquare(0.5f, { 1,1 });

    // 4) GPU 버퍼 생성 & 등록
    auto d3d = static_cast<D3D11RendererAPI*>(renderer);
    ID3D11Device* device = d3d->GetDevice();
    auto quadMesh = MeshFactory::CreateMeshFromData(device, quadData);
    MeshHandle quadHandle = d3d->GetMeshRegistry()->Register(*quadMesh);

    

    // 2.1 HLSL 컴파일
    ComPtr<ID3DBlob> vsBlob, psBlob, errBlob;
    HRESULT hr = D3DCompileFromFile(
        L"Basic.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "VSMain", "vs_5_0",
        0, 0,
        &vsBlob, &errBlob
    );
    if (FAILED(hr) && errBlob)
    {
        OutputDebugStringA((char*)errBlob->GetBufferPointer());
    }
    assert(SUCCEEDED(hr));


    // Pixel Shader
    hr = D3DCompileFromFile(
        L"Basic.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "PSMain", "ps_5_0",
        0, 0,
        &psBlob, &errBlob
    );
    assert(SUCCEEDED(hr));

    // 2.2 실제 D3D11 셰이더 객체 생성z
    ComPtr<ID3D11VertexShader>   vs;
    ComPtr<ID3D11PixelShader>    ps;
    d3d->GetDevice()->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vs);
    d3d->GetDevice()->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &ps);

    // 2.3 InputLayout 정의 (Vertex.h 구조에 맞춰서)
    D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, position),    D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, normalModel), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, offsetof(Vertex, texcoord),    D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT" , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, tangentModel),D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    ComPtr<ID3D11InputLayout> inputLayout;
    d3d->GetDevice()->CreateInputLayout(
        layoutDesc, _countof(layoutDesc),
        vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
        &inputLayout
    );

    // 5) 단일 PSO + 머티리얼 생성 (간략히)
    //    실제로는 셰이더 로딩 + 상태 설정이 필요
    auto pso = std::make_unique<PipelineState>();
    pso->m_vertexShader = vs;
    pso->m_pixelShader = ps;
    pso->m_inputLayout = inputLayout;
    PSOHandle psoHandle = d3d->GetPSORegistry()->Register(std::move(pso));

    auto layout = std::make_shared<UniformLayout>();
    layout->AddUniform("modelMatrix", sizeof(DirectX::XMMATRIX));
    layout->AddUniform("viewProj", sizeof(DirectX::XMMATRIX));
    auto material = std::make_unique<Material>(psoHandle, layout);

    // 텍스쳐 샘플러 등록

    ComPtr<ID3D11ShaderResourceView> checkerSrv =
        BinRenderer::LoadTexture(d3d->GetDevice(), "checker.png");

    TextureHandle checkerTex = d3d->GetTextureRegistry()->Register(checkerSrv);

    D3D11_SAMPLER_DESC sd{};
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    ComPtr<ID3D11SamplerState> sampler;
    d3d->GetDevice()->CreateSamplerState(&sd, &sampler);
    SamplerHandle checkerSmp = d3d->GetSamplerRegistry()->Register(sampler);

    // 머티리얼 설정 시…
    material->BindTexture(0, checkerTex);   // t0 레지스터
    material->BindSampler(0, checkerSmp);   // s0 레지스터
    auto matHandle = d3d->GetMaterialRegistry()->Register(std::move(material));

    // 6) 메시 루프
    MSG msg{};
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        

        // 7) 렌더링
        renderer->BeginFrame();

        DrawCommand cmd;
        cmd.meshHandle = quadHandle;
        cmd.materialHandle = matHandle;
        cmd.psoHandle = psoHandle;
        cmd.transform = DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f);
        cmd.viewId = 0;

        auto materialPtr = d3d->GetMaterialRegistry()->Get(matHandle);
		materialPtr->GetUniformSet().Set("modelMatrix", &cmd.transform, sizeof(cmd.transform));


        renderer->Submit(cmd);
        renderer->Submit();   // 큐 비우며 실제 드로우
        renderer->EndFrame();
        renderer->Present();
    }

    // 8) 정리
    DestroyRenderer(renderer);
    return 0;
}
