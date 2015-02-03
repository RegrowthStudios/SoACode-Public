#pragma once
#include <Vorb/graphics/gtypes.h>
#include "SphericalTerrainPatch.h"

class PlanetGenData;
class SphericalTerrainMeshManager;
class TerrainGenDelegate;

class TerrainVertex {
public:
    f32v3 position; //12
    f32v3 tangent; //24
    f32v2 texCoords; //32
    ColorRGB8 color; //35
    ui8 padding; //36
    ui8v2 normTexCoords; //38
    ui8 temperature; //39
    ui8 humidity; //40
};

class WaterVertex {
public:
    f32v3 position; //12
    f32v3 tangent; //24
    ColorRGB8 color; //27
    ui8 temperature; //28
    f32v2 texCoords; //36
    float depth; //40
};

class TerrainPatchMesher {
public:
    TerrainPatchMesher(SphericalTerrainMeshManager* meshManager,
                       PlanetGenData* planetGenData);
    ~TerrainPatchMesher();

    /// Generates mesh using heightmap
    void buildMesh(TerrainGenDelegate* data, float heightData[PATCH_HEIGHTMAP_WIDTH][PATCH_HEIGHTMAP_WIDTH][4]);

private:

    ui8 calculateTemperature(float range, float angle, float baseTemp);

    ui8 calculateHumidity(float range, float angle, float baseHum);

    void buildSkirts();

    void addWater(int z, int x, float heightData[PATCH_HEIGHTMAP_WIDTH][PATCH_HEIGHTMAP_WIDTH][4]);

    void tryAddWaterVertex(int z, int x, float heightData[PATCH_HEIGHTMAP_WIDTH][PATCH_HEIGHTMAP_WIDTH][4]);

    void tryAddWaterQuad(int z, int x);

    /// TODO: THIS IS REUSABLE
    void generateIndices(VGIndexBuffer& ibo, bool ccw);

    float computeAngleFromNormal(const f32v3& normal);

    static VGIndexBuffer m_sharedIbo; ///< Reusable CCW IBO

    // PATCH_WIDTH * 4 is for skirts
    static const int VERTS_SIZE = PATCH_SIZE + PATCH_WIDTH * 4;
    static TerrainVertex verts[VERTS_SIZE];
    static WaterVertex waterVerts[VERTS_SIZE];
    static ui16 waterIndexGrid[PATCH_WIDTH][PATCH_WIDTH];
    static ui16 waterIndices[SphericalTerrainPatch::INDICES_PER_PATCH];
    static bool waterQuads[PATCH_WIDTH - 1][PATCH_WIDTH - 1];

    PlanetGenData* m_planetGenData = nullptr; ///< Planetary data
    SphericalTerrainMeshManager* m_meshManager = nullptr;

    /// Meshing helper vars
    int m_index;
    int m_waterIndex;
    int m_waterIndexCount;
    float m_vertWidth;
    float m_radius;
    i32v3 m_coordMapping;
    f32v3 m_startPos;
    f32v2 m_coordMults;
};

