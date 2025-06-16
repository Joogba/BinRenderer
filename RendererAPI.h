
#pragma once

namespace BinRenderer {

    enum class APIType { None, D3D11, OpenGL }; // �̷� Ȯ�� ���

    struct InitParams {
        void* windowHandle;
        int width;
        int height;
    };

    class RendererAPI {
    public:
        virtual ~RendererAPI() = default;

        virtual bool Init(const InitParams& params) = 0;
        virtual void BeginFrame() = 0;
        virtual void Submit() = 0; // ��: DrawCommand ť ó��
        virtual void Submit(const DrawCommand& cmd) = 0;
        virtual void EndFrame() = 0;
        virtual void Present() = 0;
    };

    RendererAPI* CreateD3D11Renderer(); // ���丮
    void DestroyRenderer(RendererAPI* renderer);

}
