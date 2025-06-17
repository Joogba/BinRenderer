// View.h
#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>

namespace BinRenderer {

    struct View
    {
        // ·»´õ Å¸°Ù + ±íÀÌºä
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> dsv;

        // Å¬¸®¾î ¼³Á¤
        uint32_t clearFlags = 0;          // ClearFlags ºñÆ®¸¶½ºÅ©
        uint32_t clearColor = 0xff000000; // AARRGGBB
        float    clearDepth = 1.0f;
        uint8_t  clearStencil = 0;

        // ºäÆ÷Æ®
        D3D11_VIEWPORT vp = { 0,0,0,0,0,1 };
    };

} // namespace BinRenderer
