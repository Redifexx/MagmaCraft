#pragma once

namespace Magma
{

    // Screen Quad (NDC)
    constexpr float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    constexpr float cubeVertices[] = {
        // +X Face
        0.5f,  0.5f,  0.5f,   0.5f,  0.5f, -0.5f,   0.5f, -0.5f, -0.5f,   0.5f, -0.5f,  0.5f,
        // -X Face
        -0.5f,  0.5f, -0.5f,  -0.5f,  0.5f,  0.5f,  -0.5f, -0.5f,  0.5f,  -0.5f, -0.5f, -0.5f,
        // +Y Face
        -0.5f,  0.5f, -0.5f,   0.5f,  0.5f, -0.5f,   0.5f,  0.5f,  0.5f,  -0.5f,  0.5f,  0.5f,
        // -Y Face
        -0.5f, -0.5f,  0.5f,   0.5f, -0.5f,  0.5f,   0.5f, -0.5f, -0.5f,  -0.5f, -0.5f, -0.5f,
        // +Z Face
        -0.5f,  0.5f,  0.5f,   0.5f,  0.5f,  0.5f,   0.5f, -0.5f,  0.5f,  -0.5f, -0.5f,  0.5f,
        // -Z Face
        0.5f,  0.5f, -0.5f,  -0.5f,  0.5f, -0.5f,  -0.5f, -0.5f, -0.5f,   0.5f, -0.5f, -0.5f
    };

    constexpr unsigned int cubeIndices[] = {
        0, 2, 1, 2, 0, 3,       // +Xj
        4, 6, 5, 6, 4, 7,       // -X
        8, 10, 9, 10, 8, 11,    // +Y
        12, 14, 13, 14, 12, 15, // -Y
        16, 18, 17, 18, 16, 19, // +Z
        20, 22, 21, 22, 20, 23  // -Z
    };
}