// GeometryGenerator.h
#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "MeshData.h"

namespace BinRenderer {

    class GeometryGenerator {
    public:
        // Creates a unit square on the XY plane, centered at origin
        // scale: size multiplier, texScale: UV scale
        static MeshData MakeSquare(
            float scale = 1.0f,
            const glm::vec2& texScale = glm::vec2(1.0f, 1.0f));

        // Creates a grid of quads on the XY plane
        // numSlices, numStacks: subdivisions, scale: size, texScale: UV scale
        static MeshData MakeSquareGrid(
            int numSlices,
            int numStacks,
            float scale = 1.0f,
            const glm::vec2& texScale = glm::vec2(1.0f, 1.0f));

        // Additional generators (MakeSphere, MakeCylinder, etc.) can be added here
    };

} // namespace BinRenderer