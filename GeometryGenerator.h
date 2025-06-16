// GeometryGenerator.h
#pragma once

#include <vector>
#include <directxtk/SimpleMath.h>
#include "MeshData.h"

namespace BinRenderer {

    using DirectX::SimpleMath::Vector2;
    using DirectX::SimpleMath::Vector3;

    class GeometryGenerator {
    public:
        // Creates a unit square on the XY plane, centered at origin
        // scale: size multiplier, texScale: UV scale
        static MeshData MakeSquare(
            float scale = 1.0f,
            const Vector2& texScale = Vector2(1.0f, 1.0f));

        // Creates a grid of quads on the XY plane
        // numSlices, numStacks: subdivisions, scale: size, texScale: UV scale
        static MeshData MakeSquareGrid(
            int numSlices,
            int numStacks,
            float scale = 1.0f,
            const Vector2& texScale = Vector2(1.0f, 1.0f));

        // Additional generators (MakeSphere, MakeCylinder, etc.) omitted
    };

} // namespace BinCore