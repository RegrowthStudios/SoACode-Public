#pragma once
#include <vector>
#include "types.h"

struct ChunkMesh
{
    ChunkMesh() : vboID(0), vaoID(0), transVaoID(0), transVboID(0), indexSize(0), waterVboID(0), waterIndexSize(0), vecIndex(-1), distance(30.0f), needsSort(true) {}

    //***** This 84 byte block gets memcpyd from ChunkMeshData *****
    GLint pxVboOff, pxVboSize, nxVboOff, nxVboSize, pzVboOff, pzVboSize, nzVboOff, nzVboSize;
    GLint pyVboOff, pyVboSize, nyVboOff, nyVboSize, transVboSize;
    GLint highestY, lowestY, highestX, lowestX, highestZ, lowestZ;
    GLuint indexSize;
    GLuint waterIndexSize;
    //*****  End Block *****

    GLuint vboID;
    GLuint vaoID;
    GLuint transVboID;
    GLuint transVaoID;
    GLuint waterVboID;
    float distance;
    glm::ivec3 position;
    int vecIndex;
    bool inFrustum;
    bool needsSort;

    //*** Transparency info for sorting ***
    GLuint transIndexID;
    std::vector <i8v3> transQuadPositions;
    std::vector <ui32> transQuadIndices;
    
};

class ChunkRenderer {
public:
    static void draw(const ChunkMesh *CMI, const glm::dvec3 &PlayerPos, const glm::mat4 &VP);
    static void drawTransparentBlocks(const ChunkMesh *CMI, const glm::dvec3 &playerPos, const glm::mat4 &VP);
    static void drawSonar(const ChunkMesh *CMI, const glm::dvec3 &PlayerPos, const glm::mat4 &VP);
    static void drawWater(const ChunkMesh *CMI, const glm::dvec3 &PlayerPos, const glm::mat4 &VP);

    static void bindVao(ChunkMesh *CMI);
    static void bindNonBlockVao(ChunkMesh *CMI);
};