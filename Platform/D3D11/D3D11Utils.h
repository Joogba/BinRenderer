#pragma once
#include <d3d11.h>
#include "Core/Format.h"
#include "Core/PrimitiveTopology.h"

namespace Binrenderer::D3D11Utils
{
	DXGI_FORMAT ToDXGIFormat(BinRenderer::Format fmt);
	D3D11_PRIMITIVE_TOPOLOGY ToD3D11PrimitiveTopology(BinRenderer::PrimitiveTopology topo);
}