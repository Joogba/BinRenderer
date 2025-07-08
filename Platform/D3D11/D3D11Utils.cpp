#include "D3D11Utils.h"
#include "D3D11Utils.h"

namespace BinRenderer::D3D11Utils {

    DXGI_FORMAT ToDXGIFormat(BinRenderer::Format fmt) {
        switch (fmt) {
        case Format::R8G8B8A8_UNorm: return DXGI_FORMAT_R8G8B8A8_UNORM;
            // ...필요한 포맷만 
        default: return DXGI_FORMAT_UNKNOWN;
        }
    }

    D3D11_PRIMITIVE_TOPOLOGY ToD3D11PrimitiveTopology(BinRenderer::PrimitiveTopology topo) {
        switch (topo) {
        case PrimitiveTopology::TriangleList: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            // ...필요한 토폴로지만
        default: return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
        }
    }

}