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

enum class CubeFace { TOP, LEFT, RIGHT, FRONT, BACK, BOTTOM };

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
    f32v3 boundingBox;
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
    f32v3 boundingBox;
};


#define PATCH_WIDTH 5

class SphericalTerrainData {
public:
    friend class SphericalTerrainComponent;

    SphericalTerrainData(f64 radius,
                         f64 patchWidth) :
        m_radius(radius),
        m_patchWidth(patchWidth) {
        // Empty
    }

    const f64& getRadius() const { return m_radius; }
private:
    f64 m_radius;
    f64 m_patchWidth; ///< Width of a patch in KM
};

class SphericalTerrainPatch {
public:
    SphericalTerrainPatch() { };
    ~SphericalTerrainPatch();
    
    void init(const f64v2& gridPosition,
              CubeFace cubeFace,
              const SphericalTerrainData* sphericalTerrainData,
              f64 width);

    void update(const f64v3& cameraPos);

    void destroy();

    // Temporary
    void draw(const f64v3& cameraPos, const f32m4& VP, vg::GLProgram* program);

    bool hasMesh() const { return (m_vbo != 0); }
    bool isRenderable() const;

private:
    void destroyMesh();
    void generateMesh(float heightData[PATCH_WIDTH][PATCH_WIDTH]);

    f64v2 m_gridPosition = f64v2(0.0);
    f64v3 m_worldPosition = f64v3(0.0);
    f32v3 m_boundingBox;
    f64 m_distance = 1000000000.0;
    int m_lod = 0;
    CubeFace m_cubeFace;

    f64 m_width = 0.0;

    ui32 m_vao = 0;
    ui32 m_vbo = 0;
    ui32 m_ibo = 0;

    const SphericalTerrainData* m_sphericalTerrainData = nullptr;
    SphericalTerrainPatch* m_children = nullptr;
};