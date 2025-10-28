#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <DirectXTK/WICTextureLoader.h> 

namespace BinRenderer {

    // ���� ���(wstring)���� SRV ����
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
            // �α� �Ǵ� ���� ó�� ���⿡ �߰�
            return nullptr;
        }
        return srv;
    }

    // string ���� �����ε�
    inline Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> LoadTexture(
        ID3D11Device* device,
        const std::string& filePath
    ) {
        std::wstring wfile(filePath.begin(), filePath.end());
        return LoadTexture(device, wfile);
    }

} // namespace BinRenderer
