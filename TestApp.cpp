// TestApp.cpp
#include <windows.h>
#include <cassert>
#include <d3d11.h>
#include <d3dcompiler.h>

#include "D3D11RendererAPI.h"
#include "GeometryGenerator.h"
#include "MeshFactory.h"
#include "DrawCommand.h"
#include "Handle.h"
 
using namespace BinRenderer;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (WM_SIZE == msg)
    {
        int w = LOWORD(lParam), h = HIWORD(lParam);
        // ������ �������� (�߰� API �ʿ� �� ����)
    }
    if (WM_DESTROY == msg)
    {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, PSTR, int nCmdShow)
{
    // 1) ������ ����
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
    ShowWindow(hWnd, nCmdShow);

    // 2) ������ ���� & �ʱ�ȭ
    RendererAPI* renderer = CreateD3D11Renderer();
    InitParams params{ hWnd, 1280, 720 };
    assert(renderer->Init(params) && "Init failed");

    // 2.1) ī�޶�(�䡤����) ��� ����
    using namespace DirectX;
       // ������ ���� ������ Z�� -3 ������ ��ġ���� �ٶ󺸱�
    XMVECTOR eye = XMVectorSet(1.0f, 0.0f, -3.0f, 0.0f);
    XMVECTOR at = XMVectorSet(1.0f, 0.0f, 1.0f, 0.0f);
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMMATRIX view = XMMatrixLookAtLH(eye, at, up);
    XMMATRIX proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, width / (float)height, 0.1f, 100.0f);
    renderer->SetViewProj(view, proj);

    // 3) ���� ���� (�� ����)
    MeshData quadData = GeometryGenerator::MakeSquare(0.5f, { 1,1 });

    // 4) GPU ���� ���� & ���
    auto d3d = static_cast<D3D11RendererAPI*>(renderer);
    ID3D11Device* device = d3d->GetDevice();
    auto quadMesh = MeshFactory::CreateMeshFromData(device, quadData);
    MeshHandle quadHandle = d3d->GetMeshRegistry()->Register(*quadMesh);

    // 2.1 HLSL ������
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

    // 2.2 ���� D3D11 ���̴� ��ü ����z
    ComPtr<ID3D11VertexShader>   vs;
    ComPtr<ID3D11PixelShader>    ps;
    d3d->GetDevice()->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vs);
    d3d->GetDevice()->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &ps);

    // 2.3 InputLayout ���� (Vertex.h ������ ���缭)
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

    // 5) ���� PSO + ��Ƽ���� ���� (������)
    //    �����δ� ���̴� �ε� + ���� ������ �ʿ�
    auto pso = std::make_unique<PipelineState>();
    pso->m_vertexShader = vs;
    pso->m_pixelShader = ps;
    pso->m_inputLayout = inputLayout;
    PSOHandle psoHandle = d3d->GetPSORegistry()->Register(std::move(pso));

    auto layout = std::make_shared<UniformLayout>();
    layout->AddUniform("modelMatrix", sizeof(DirectX::XMMATRIX));
    layout->AddUniform("viewProj", sizeof(DirectX::XMMATRIX));
    auto material = std::make_unique<Material>(psoHandle, layout);
    MaterialHandle matHandle = d3d->GetMaterialRegistry()->Register(std::move(material));

    // 6) �޽� ����
    MSG msg{};
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        // 7) ������
        renderer->BeginFrame();

        DrawCommand cmd;
        cmd.meshHandle = quadHandle;
        cmd.materialHandle = matHandle;
        cmd.psoHandle = psoHandle;
        cmd.transform = DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f);

        // modelMatrix ����
        {
            auto mat = d3d->GetMaterialRegistry()->Get(cmd.materialHandle);
            mat->GetUniformSet().Set("modelMatrix", &cmd.transform, sizeof(cmd.transform));
            // viewProj�� Submit()���� �ڵ����� ä����
        }

        auto materialPtr = d3d->GetMaterialRegistry()->Get(matHandle);
        materialPtr->GetUniformSet().Set(
            "modelMatrix",
            &cmd.transform,
            sizeof(cmd.transform)
        );

        renderer->Submit(cmd);
        renderer->Submit();   // ť ���� ���� ��ο�
        renderer->EndFrame();
        renderer->Present();
    }

    // 8) ����
    DestroyRenderer(renderer);
    return 0;
}
