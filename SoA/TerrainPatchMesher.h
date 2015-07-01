///
/// TerrainPatchMesher.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 3 Feb 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Creates terrain patch meshes
///

#pragma once

#ifndef TerrainPatchMesher_h__
#define TerrainPatchMesher_h__

#include <Vorb/graphics/gtypes.h>
#include "TerrainPatchMesh.h"

struct PlanetGenData;
class TerrainPatchMeshManager;

class TerrainPatchMesher {
public:
    /// Generates shared index buffer.
    static void generateIndices();
    static void destroyIndices();

    /// Generates mesh using heightmap
    void generateMeshData(TerrainPatchMesh* mesh, const PlanetGenData* planetGenData,
                          const f32v3& startPos, WorldCubeFace cubeFace,
                          float width, f32 heightData[PATCH_WIDTH][PATCH_WIDTH][4],
                          f32v3 positionData[PATCH_WIDTH][PATCH_WIDTH]);

    static void uploadMeshData(TerrainPatchMesh* mesh);

    static const int VERTS_SIZE = PATCH_SIZE + PATCH_WIDTH * 4; ///< Number of vertices per patch
private:

    /// Calculates temperature based on angle with equator
    /// @param range: The range to scale between
    /// @angle: Angle from equator
    /// @param baseTemp: Base temperature at equator
    ui8 calculateTemperature(float range, float angle, float baseTemp);

    /// Calculates humidity based on angle with equator
    /// @param range: The range to scale between
    /// @angle: Angle from equator
    /// @param baseTemp: Base humidity at equator
    ui8 calculateHumidity(float range, float angle, float baseHum);

    /// Builds the skirts for a patch
    void buildSkirts();

    /// Adds water at a given point
    /// @param z: Z position
    /// @Param x: X position
    /// @param heightData: The heightmap data
    void addWater(int z, int x, float heightData[PATCH_WIDTH][PATCH_WIDTH][4]);

    /// Tries to add a water vertex at a given spot if one doesn't exist
    /// @param z: Z position
    /// @param x: X position
    /// @param heightData: The heightmap data
    void tryAddWaterVertex(int z, int x, float heightData[PATCH_WIDTH][PATCH_WIDTH][4]);

    /// Tries to add a quad at a given spot if one doesnt exist
    /// @param z: Z position
    /// @param x: X position
    void tryAddWaterQuad(int z, int x);

    /// Computes angle with the equator based on the normal of position
    /// @param normal: Normalized position on sphere
    f32 computeAngleFromNormal(const f32v3& normal);

    static VGIndexBuffer m_sharedIbo; ///< Reusable CCW IBO

    // PATCH_WIDTH * 4 is for skirts

    TerrainVertex verts[VERTS_SIZE]; ///< Vertices for terrain mesh
    WaterVertex waterVerts[VERTS_SIZE]; ///< Vertices for water mesh
    ui16 waterIndexGrid[PATCH_WIDTH][PATCH_WIDTH]; ///< Caches water indices for reuse
    ui16 waterIndices[PATCH_INDICES]; ///< Buffer of indices to upload
    bool waterQuads[PATCH_WIDTH - 1][PATCH_WIDTH - 1]; ///< True when a quad is present at a spot

    const PlanetGenData* m_planetGenData = nullptr; ///< Planetary data
    TerrainPatchMeshManager* m_meshManager = nullptr; ///< Manages the patch meshes

    /// Meshing helper vars
    int m_index;
    int m_waterIndex;
    int m_waterIndexCount;
    f32 m_vertWidth;
    f32 m_radius;
    i32v3 m_coordMapping;
    f32v3 m_startPos;
    f32v2 m_coordMults;
    bool m_isSpherical;
    WorldCubeFace m_cubeFace;
};

#endif // TerrainPatchMesher_h__
