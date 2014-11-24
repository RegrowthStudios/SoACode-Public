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

extern vector<TerrainVertex> tvboVerts; //this is bigger that it needs to be but whatever
extern vector<GLushort> lodIndices;

class Camera;

struct TerrainBuffers{
    TerrainBuffers() : vboID(0), vaoID(0), vboIndexID(0), treeVboID(0), indexSize(0), treeIndexSize(0), vecIndex(-1), inFrustum(0), distance(0.0){}
    GLuint vboID;
    GLuint vaoID;
    GLuint vboIndexID;
    GLuint treeVboID;
    int indexSize;
    int treeIndexSize;
    int worldX, worldY, worldZ;
    int drawX, drawY, drawZ, vecIndex;
    int cullRadius;
    double distance;
    bool inFrustum;
    glm::vec3 boundingBox;
};

struct TerrainMeshMessage{
    TerrainMeshMessage() : terrainBuffers(NULL), verts(NULL), indices(NULL), treeVerts(NULL), index(0), indexSize(0), treeIndexSize(0), face(0){}
    TerrainBuffers* terrainBuffers;
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
    bool debug;
    int X, Y, Z;
    int drawX, drawY, drawZ;
    int cX, cY, cZ;
    int worldX, worldY, worldZ;
    int radius;
    int scaledRadius;
    int face;
    int step;
    int lodMapStep;
    int lodMapSize;
    short detailLevel;
    bool hasSkirt;
    bool hasProperDist;
    bool hasBoundingBox;
    int vecIndex;
    int updateVecIndex;
    int distance;
    int width;
    float cullRadius;
    bool drawTrees;

    HeightData *lodMap;
    TerrainPatch* lods[4];
    TerrainPatch *parent;
    TerrainBuffers *terrainBuffers;
    volatile bool hasBuffers;
    //0 = no update needed. 1 = update needed 2 = child deletion needed 
    int updateCode;
    int childNum;

    TerrainPatch(int lodWidth);
    ~TerrainPatch();

    void NullifyBuffers();
    void ClearBuffers();
    void ClearTreeBuffers();
    void ClearLODMap();

    void DeleteChildren();
    //itinializes the LOD and computes distance
    void Initialize(int x, int y, int z, int wx, int wy, int wz, int Radius, int Face, TerrainPatch *Parent = NULL, int ChildNum = -1, int initialDetail = -1);
    static void Draw(TerrainBuffers *tb, const Camera* camera, const glm::dvec3 &PlayerPos, const glm::dvec3 &rotPlayerPos, const glm::mat4 &VP, GLuint mvpID, GLuint worldOffsetID, bool onPlanet);
    static void DrawTrees(TerrainBuffers *tb, const vg::GLProgram* program, const glm::dvec3 &PlayerPos, const glm::mat4 &VP);
    static bool CheckHorizon(const glm::dvec3 &PlayerPoss, const glm::dvec3 &ClosestPoint);
    //inline void ExtractChildData();
    bool CreateMesh();
    
    inline void CreateChildren(int wx, int wy, int wz, int initialDetail = -1);
    int update(int wx, int wy, int wz);
    void CalculateDetailLevel(double dist, int threshold);
    void SortChildren();

    glm::vec3 worldNormal;
    glm::vec3 boundingBox;
    glm::dvec3 closestPoint;
};

class CloseTerrainPatch{
public:
    int X, Y, Z;
    int cX, cY, cZ;
  //  FaceData faceData;
    int step;
    int lodMapStep;
    int lodMapSize;
    short detailLevel;
    int indexSize;
    int treeIndexSize;
    bool hasMesh;
    bool hasSkirt;
    bool hasProperDist;
    bool hasBoundingBox;
    int vecIndex;
    int updateVecIndex;
    int distance;
    int width;
    float cullRadius;
    bool drawTrees;
    bool waitForMesh;

    HeightData *lodMap;
    CloseTerrainPatch* lods[4];
    CloseTerrainPatch *parent;
    GLuint vboID;
    GLuint vaoID;
    GLuint vboIndexID;
    GLuint treeVboID;
    //0 = no update needed. 1 = update needed 2 = child deletion needed 
    int updateCode;
    int childNum;

    CloseTerrainPatch(int lodWidth);
    ~CloseTerrainPatch();

    void ClearBuffers();
    void ClearTreeBuffers();
    void ClearLODMap();

    void DeleteChildren();
    //itinializes the LOD and computes distance
    void Initialize(int x, int y, int z, int wx, int wy, int wz, CloseTerrainPatch *Parent = NULL, int ChildNum = -1, int initialDetail = -1);
    void Draw(glm::dvec3 &PlayerPos, glm::dvec3 &rotPlayerPos, glm::mat4 &VP, GLuint mvpID, GLuint worldOffsetID, bool onPlanet);
    void DrawTrees(glm::dvec3 &PlayerPos, glm::mat4 &VP);
    //inline void ExtractChildData();
    bool CreateMesh();

    inline void CreateChildren(int wx, int wy, int wz, int initialDetail = -1);
    int update(int wx, int wy, int wz);
    void CalculateDetailLevel(double dist, int threshold);
    void SortChildren();

    glm::vec3 boundingBox;
    glm::dvec3 closestPoint;
};
