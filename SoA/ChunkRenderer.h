#pragma once
#include <vector>
#include "types.h"
#include "ChunkMesh.h"

class ChunkRenderer {
public:
    static void draw(const ChunkMesh *CMI, const glm::dvec3 &PlayerPos, const glm::mat4 &VP);
    static void drawTransparentBlocks(const ChunkMesh *CMI, const glm::dvec3 &playerPos, const glm::mat4 &VP);
    static void drawCutoutBlocks(const ChunkMesh *CMI, const glm::dvec3 &playerPos, const glm::mat4 &VP);
    static void drawSonar(const ChunkMesh *CMI, const glm::dvec3 &PlayerPos, const glm::mat4 &VP);
    static void drawWater(const ChunkMesh *CMI, const glm::dvec3 &PlayerPos, const glm::mat4 &VP);

    static void bindTransparentVao(ChunkMesh *CMI);
    static void bindCutoutVao(ChunkMesh *CMI);
    static void bindVao(ChunkMesh *CMI);
    
};