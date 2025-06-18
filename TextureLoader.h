#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <DirectXTK/WICTextureLoader.h> 

namespace BinRenderer {

    // 파일 경로(wstring)에서 SRV 생성
    inline Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> LoadTexture(
        ID3D11Device* device,
        const std::wstring& filePath
    ) {
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
        HRESULT hr = DirectX::CreateWICTextureFromFile(
            device, filePath.c_str(),
            nullptr,
            srv.GetAddressOf()
        );
        if (FAILED(hr)) {
            // 로깅 또는 예외 처리 여기에 추가
            return nullptr;
        }
        return srv;
    }

    // string 버전 오버로드
    inline Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> LoadTexture(
        ID3D11Device* device,
        const std::string& filePath
    ) {
        std::wstring wfile(filePath.begin(), filePath.end());
        return LoadTexture(device, wfile);
    }

} // namespace BinRenderer
