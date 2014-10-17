#pragma once
#include <vector>
#include "types.h"
#include "ChunkMesh.h"
#include "GLProgram.h"

class ChunkRenderer {
public:
    static void drawBlocks(const ChunkMesh *CMI, const vcore::GLProgram* program, const glm::dvec3 &PlayerPos, const glm::mat4 &VP);
    static void drawTransparentBlocks(const ChunkMesh *CMI, const vcore::GLProgram* program, const glm::dvec3 &playerPos, const glm::mat4 &VP);
    static void drawCutoutBlocks(const ChunkMesh *CMI, const vcore::GLProgram* program, const glm::dvec3 &playerPos, const glm::mat4 &VP);
    static void drawWater(const ChunkMesh *CMI, const vcore::GLProgram* program, const glm::dvec3 &PlayerPos, const glm::mat4 &VP);

    static void bindTransparentVao(ChunkMesh *CMI);
    static void bindCutoutVao(ChunkMesh *CMI);
    static void bindVao(ChunkMesh *CMI);
    static void bindWaterVao(ChunkMesh *CMI);
    
};