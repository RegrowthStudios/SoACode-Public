#pragma once
#include "global.h"

#include "Vorb.h"
#include "IVoxelMapper.h"

// TODO: Remove This
using namespace std;

const int FREEZETEMP = 50;

// TODO(Ben): This is absolutely awful
class TerrainGenerator
{
public:
    enum DefaultColorMaps { BIOME = 0, WATER = 1 };

    TerrainGenerator();

    void getColorMapColor(ColorRGB8& color, int temperature, int rainfall);

    double findnoise2(double x,double z);
    double interpolate1(double a,double b,double x);
    double noise(double x,double z);
    void setVoxelMapping(vvoxel::VoxelMapData* voxelMapData, int Radius, int pScale);
    void SetLODFace(int Ipos, int Jpos, int Rpos, int Radius, int idir = 1, int jdir = 1, float pscale = 1.0f);
    void GenerateHeightMap(HeightData *lodMap, int icoord, int jcoord, int size, int iend, int jend, int step, int iskip, bool *hasWater = NULL);
    double GenerateSingleHeight(int x, int y, int z);
    //void GenerateVoxelMap(vector <vector <HeightData > > *heightMap, int x, int y, int z, int width, int height, int iStart=0, int jStart=0);
    void AddNoiseFunction(double persistence, double frequency, int octaves, double lowBound, double upBound, double Scale, int type, bool isModifier);
    void ClearFunctions();
    void SetPreturbNoiseFunction(double persistence, double frequency, int octaves, double lowBound, double upBound, double Scale, int type);
    void SetTributaryNoiseFunction(double persistence, double frequency, int octaves, double lowBound, double upBound, double Scale, int type);
    void SetRiverNoiseFunction(double persistence, double frequency, int octaves, double lowBound, double upBound, double Scale, int type);
    void SetBiomeOffsetNoiseFunction(double persistence, double frequency, int octaves, double lowBound, double upBound, double Scale, int type);
    void SetTemperatureNoiseFunction(double persistence, double frequency, int octaves, double lowBound, double upBound, double Scale, int type);
    void SetRainfallNoiseFunction(double persistence, double frequency, int octaves, double lowBound, double upBound, double Scale, int type);
    void CalculateSurfaceDensity(double X, double Y, double Z, double *SurfaceDensity, int size, int offset, int oct, double pers, double freq);
    void CalculateStalactiteDensity(glm::ivec3 pos, double *CaveDensity, int size, int offset, int oct, double pers, double freq);
    void CalculateCaveDensity(glm::ivec3 pos, double *CaveDensity, int size, int offset, int oct, double pers, double freq);
    inline double GetTemperature(double x, double y, double z);
    inline double GetRainfall(double x, double y, double z);
    double GetRiverNoiseHeight(double &noiseHeight, NoiseInfo *noisef, double x, double y, double z, GLuint &flags);
    void GetTributaryNoiseHeight(double &noiseHeight, NoiseInfo *noisef, double x, double y, double z, GLuint &flags, double rh);
    void GetNoiseHeight(double &noiseHeight, NoiseInfo *noisef, double x, double y, double z, GLuint &flags);
    void GetBaseBiome(int x, int y, int z, Biome **b, int h);
    void InfluenceTerrainWithBiome(Biome *b, HeightData *hv, double x, double y, double z, double &temp, double &rain, double &height, double &sandDepth, GLuint &flags, double terrainMult);

    void postProcessHeightmap(HeightData heightData[CHUNK_LAYER]);

    int numFunctions;

    int BiomeMap[256][256]; //[rainfall][temperature]

    void SetHeightModifier(double hm){ heightModifier = hm; }
    void SetDefaultTempRain(double tmp, double rn){ defaultTemp = tmp; defaultRain = rn; }

    // For the useMapColor blockdata property
    ColorRGB8* getColorMap(const nString& name);
    std::map <nString, ui32> blockColorMapLookupTable;
    std::vector <ColorRGB8*> blockColorMaps;

//private:
    double TileFunc(double x, double y, double w, double h, double dt);
    double heightModifier;
    double defaultTemp, defaultRain;
    vector <NoiseInfo> noiseFunctions;
    NoiseInfo *temperatureNoiseFunction;
    NoiseInfo *rainfallNoiseFunction;
    NoiseInfo *biomeOffsetNoiseFunction;
    NoiseInfo *riverNoiseFunction;
    NoiseInfo *tributaryNoiseFunction;
    NoiseInfo *preturbNoiseFunction;
    int iPos, jPos, rPos, radius;
    int iDir, jDir;
    float scale;
};

const int NumTerrainFunctions = 11;
const string TerrainFunctionNames[NumTerrainFunctions] = { "Default", "Small Mts.", "Large Mts.+Lakes", "Hills+Plateaus", "Seaside Cliffs",
    "Ridged", "Billowy Hills", "Jagged Cliffs", "Huge Mts.", "Volcano", "Canyons" };

extern string TerrainFunctionHelps[NumTerrainFunctions];