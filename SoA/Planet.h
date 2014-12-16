#pragma once
#include <list>
#include <set>
#include <deque>
#include <queue>

#include "OpenGLStructs.h"
#include "Texture2d.h"
#include "WorldStructs.h"

#define P_TOP 0
#define P_LEFT 1
#define P_RIGHT 2
#define P_FRONT 3
#define P_BACK 4
#define P_BOTTOM 5

struct TreeType;
struct PlantType;
class Camera;

struct AtmosphereProperties {
public:
    f32 rayleighConstant;
    f32 rayleighScaleDepth;
    f32 mieConstant;
    f32 sunIntensity;
    f32 mieAsymmetry;
    f32v3 lightWavelengths;
};

class Atmosphere {
public:
    Atmosphere();

    void initialize(nString filePath, f32 PlanetRadius);
    void loadProperties(nString filePath);
    void draw(f32 theta, const f32m4& MVP, f32v3 lightPos, const f64v3& ppos);

    std::vector<ColorVertex> vertices;
    std::vector<ui16> indices;
    ui32 vboID, vbo2ID, vboIndexID, vboIndexID2;
    ui32 indexSize;
    f64 radius;
    f64 planetRadius;

    f32 m_fWavelength[3], m_fWavelength4[3];
    f32 m_Kr;        // Rayleigh scattering constant
    f32 m_Km;        // Mie scattering constant
    f32 m_ESun;        // Sun brightness constant
    f32 m_g;        // The Mie phase asymmetry factor
    f32 m_fExposure;
    f32 m_fRayleighScaleDepth;
    f32 fSamples;
    i32 nSamples;
    i32 debugIndex;
};

class Planet {
public:
    Planet();
    ~Planet();

    void clearMeshes();
    void initialize(nString filePath);
    void initializeTerrain(const f64v3& startPosition);
    void destroy();

    void loadData(nString filePath, bool ignoreBiomes);
    void saveData();

    void draw(f32 theta, const Camera* camera, f32v3 lightPos, f32 sunVal, f32 fadeDistance, bool connectedToPlanet);
    void drawTrees(const f32m4& VP, const f64v3& PlayerPos, f32 sunVal);
    void drawGround(f32 theta, const Camera* camera, const f32m4& VP, f32v3 lightPos, const f64v3& PlayerPos, const f64v3& rotPlayerPos, float fadeDistance, bool onPlanet);

    void updateLODs(f64v3& worldPosition, ui32 maxTicks);
    void rotationUpdate();
    void flagTerrainForRebuild();
    void sortUpdateList();

    void saveProperties(nString filePath);
    void loadProperties(nString filePath);

    f64 getGravityAccel(f64 dist);
    f64 getAirFrictionForce(f64 dist, f64 velocity);
    f64 getAirDensity(f64 dist);

    i32 radius;
    i32 scaledRadius;
    i32 solarX, solarY, solarZ;
    i32 facecsGridWidth;
    i32 baseTemperature;
    i32 baseRainfall;

    f32 minHumidity, maxHumidity;
    f32 minCelsius, maxCelsius;

    f64 axialZTilt;
    f64 gravityConstant;
    f64 volume;
    f64 mass;
    f64 density;
    f64 rotationTheta;
    f64v3 northAxis;
    f32m4 rotationMatrix;
    f32m4 invRotationMatrix;

    // GM/r2
    std::vector< std::vector<class TerrainPatch*> > faces[6];
    std::vector<TerrainPatch*> LODUpdateList;
    std::vector<struct TerrainBuffers*> drawList[6];

    //        double axialZTilt[66];

    void clearBiomes();
    void addBaseBiome(Biome* baseBiome, i32 mapColor);
    void addMainBiome(Biome* mainBiome);
    void addChildBiome(Biome* childBiome);

    vg::Texture biomeMapTexture;
    vg::Texture colorMapTexture;
    vg::Texture sunColorMapTexture;
    vg::Texture waterColorMapTexture;

    i32 bindex;
    std::map<i32, Biome*> baseBiomesLookupMap;
    std::vector<Biome*> allBiomesLookupVector;
    std::vector<Biome*> mainBiomesVector;
    std::vector<Biome*> childBiomesVector;

    std::vector<NoiseInfo*> floraNoiseFunctions;
    NoiseInfo* stormNoiseFunction;
    NoiseInfo* sunnyCloudyNoiseFunction;
    NoiseInfo* cumulusNoiseFunction;

    std::vector<TreeType*> treeTypeVec;
    std::vector<PlantType*> floraTypeVec;

    i32 maximumDepth;
    std::vector<ui16> rockLayers;

    std::map<nString, i32> treeLookupMap;
    std::map<nString, i32> floraLookupMap;

    Atmosphere atmosphere;

    nString dirName;
    nString biomeMapFileName;
    nString colorMapFileName;
    nString waterColorMapFileName;

    f32q axisQuaternion;
    f32q rotateQuaternion;
};
