///
/// NoiseShaderCode.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 19 Dec 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Shader code for simplex noise
///

#pragma once

#ifndef NoiseShaderCode_h__
#define NoiseShaderCode_h__


#pragma region Shader Code
const nString NOISE_SRC_VERT = R"(
// Inputs
in vec2 vPosition;

// Outputs
out vec2 fPos;

void main() {
    fPos = (vPosition.xy + 1.0) * 0.5;
    gl_Position = vec4(vPosition, 0, 1);
}
)";

//
// Description : Array and textureless GLSL 2D/3D/4D simplex 
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : ijm
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
// 

const nString N_HEIGHT = "pOutput.r";
const nString N_TEMP = "pOutput.g";
const nString N_HUM = "pOutput.b";
const nString N_BIOME = "pOutput.a";

const nString N_TEMP_HUM_V2 = "vec2(" + N_TEMP + "," + N_HUM + ")";

const nString NOISE_SRC_FRAG = R"(
// Uniforms
uniform sampler2D unBaseBiomes;
uniform sampler2DArray unBiomes;
uniform vec3 unCornerPos = vec3(0.0);
uniform vec2 unCoordMults = vec2(1.0);
uniform ivec3 unCoordMapping = ivec3(0);
uniform float unPatchWidth = 10.0;
uniform float unRadius;

// Inputs
in vec2 fPos;

// Outputs
out vec4 pOutput;

#include "Shaders/Noise/snoise3.glsl"
#include "Shaders/Noise/Cellular/cellular3D.glsl"

void main() {
float total;
    float amplitude;
    float maxAmplitude;
    float frequency;
    float tmp;

    pOutput.a = 0.0f;

    vec3 pos;
    pos[unCoordMapping.x] = unCornerPos.x + fPos.x * unPatchWidth * unCoordMults.x;
    pos[unCoordMapping.y] = unCornerPos.y;
    pos[unCoordMapping.z] = unCornerPos.z + fPos.y * unPatchWidth * unCoordMults.y;
    pos = normalize(pos) * unRadius;
)";
#pragma endregion

#endif // NoiseShaderCode_h__
