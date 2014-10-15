#pragma once

class SpriteBatchShader {
public:
    // Program ID
    ui32 program;
    // World Transform Uniform ID
    ui32 unWorld;
    // Camera Matrix Uniform ID
    ui32 unVP;
    // Texture Uniform ID
    ui32 unTexture;
};