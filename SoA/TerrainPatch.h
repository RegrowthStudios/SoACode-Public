#pragma once
#include "Frustum.h"
#include "TerrainGenerator.h"

#include "GLProgram.h"

const int scale = (int)planetScale;

const int maxVertexWidth = 32;
const int DetailLevels = 14;

extern int lodDetailLevels[DetailLevels+3];
extern int lodDistanceLevels[DetailLevels];

extern int WaterIndexMap[(maxVertexWidth + 3)*(maxVertexWidth + 3) * 2];
extern int MakeWaterQuadMap[(maxVertexWidth + 3)*(maxVertexWidth + 3)];


class Camera;

class TerrainMesh{
public:
    GLuint vboID = 0;
    GLuint vaoID = 0;
    GLuint vboIndexID = 0;
    GLuint treeVboID = 0;
    int indexSize = 0;
    int treeIndexSize = 0;
    int worldX, worldY, worldZ;
    int drawX, drawY, drawZ, vecIndex = -1;
    int cullRadius;
    double distance = 0.0;
    bool inFrustum = false;
    glm::vec3 boundingBox;
};

class TerrainMeshMessage{
public:
    TerrainMeshMessage() : terrainBuffers(NULL), verts(NULL), indices(NULL), treeVerts(NULL), index(0), indexSize(0), treeIndexSize(0), face(0){}
    TerrainMesh* terrainBuffers;
    TerrainVertex *verts;
    GLushort *indices;
    TreeVertex *treeVerts;
    int face;
    int index;
    int indexSize;
    int treeIndexSize;
    int worldX, worldY, worldZ;
    int drawX, drawY, drawZ, vecIndex;
    int cullRadius;
    glm::vec3 boundingBox;
};

class TerrainPatch{
public:
 
};