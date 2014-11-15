#pragma once
#include <queue>

#include "Constants.h"
#include "OpenGLStructs.h"
#include "Texture2d.h"
#include "Timing.h"

extern MultiplePreciseTimer globalMultiplePreciseTimer; ///< For easy global benchmarking

extern class Item *ObjectList[OBJECT_LIST_SIZE];

const int UNLOADED_HEIGHT = INT_MAX; //sentinalized height. Nobody should get this high. If they do, damn.

struct FixedSizeBillboardVertex{
    glm::vec3 pos;
    GLubyte uv[2];
};

//TODO(Ben): Make this work again
class Marker{
public:
    glm::dvec3 pos;
    ColorRGBA8 color;
    int num;
    double dist;
    string name;

    vg::Texture distText;
    vg::Texture nameTex;

    Marker(const glm::dvec3 &Pos, string Name, const glm::vec3 Color);
    void Draw(glm::mat4 &VP, const glm::dvec3 &playerPos);
};

struct NoiseInfo
{
    NoiseInfo(){
        memset(this, 0, sizeof(NoiseInfo));
        modifier = NULL;
        name = "UNNAMED";
    }
    ~NoiseInfo(){
        if (modifier) delete modifier;
        modifier = NULL;
    }
    double persistence;
    double frequency;
    double lowBound;
    double upBound;
    double scale;
    GLint octaves;
    GLint composition;
    NoiseInfo *modifier;
    string name;
    GLint type;
};

struct BiomeTree
{
    BiomeTree(GLfloat prob, GLint index) : probability(prob), 
                                           treeIndex(index)
    {
    }
    GLfloat probability;
    GLint treeIndex;
};

struct BiomeFlora
{
    BiomeFlora(GLfloat prob, GLint index) : probability(prob), 
                                           floraIndex(index)
    {
    }
    GLfloat probability;
    GLint floraIndex;
};

struct Biome
{
    Biome();

    GLint vecIndex;
    GLint hasAltColor;
    GLint looseSoilDepth; //TO ADD
    GLint isBase; //ISBASE MUST BE LAST GLINT
    GLubyte r, g, b, padding;
    GLushort surfaceBlock;
    GLushort underwaterBlock;
    GLushort beachBlock;
    GLushort padding2; //PADDING2 MUST BE LAST GLushort
    GLfloat treeChance;
    GLfloat applyBiomeAt;
    GLfloat lowTemp, highTemp, tempSlopeLength;
    GLfloat lowRain, highRain, rainSlopeLength;
    GLfloat maxHeight, maxHeightSlopeLength;
    GLfloat minTerrainMult; //MINTERRAINMULT MUST BE LAST GLfloat
    string name;
    string filename;

    GLushort surfaceLayers[SURFACE_DEPTH]; //stores the top 50 layers corresponding to this biome
    NoiseInfo distributionNoise;
    vector <NoiseInfo> terrainNoiseList;
    vector <Biome*> childBiomes;
    vector <BiomeTree> possibleTrees;
    vector <BiomeFlora> possibleFlora;
};

//flags
const int PLATEAU = 0x1;
const int VOLCANO = 0x2;
const int TOOSTEEP = 0x4;


struct LoadData
{
    LoadData()
    {
    }
    LoadData(struct HeightData *hmap, class TerrainGenerator *gen)
    {
        heightMap = hmap;
        generator = gen;
    }
    
    inline void init(HeightData *hmap, TerrainGenerator *gen)
    {
        generator = gen;
    }

    HeightData *heightMap;
    TerrainGenerator *generator;
};

struct HeightData
{
    GLint height;
    GLint temperature;
    GLint rainfall;
    GLint snowDepth;
    GLint sandDepth;
    GLint flags;
    GLubyte depth;
    Biome *biome = nullptr;
    GLushort surfaceBlock;
};

struct MineralData
{
    MineralData(GLint btype, GLint startheight, float startchance, GLint centerheight, float centerchance, GLint endheight, float endchance, GLint minsize, GLint maxsize)
    {
        blockType = btype;
        startHeight = startheight;
        startChance = startchance;
        endHeight = endheight;
        endChance = endchance;
        centerHeight = centerheight;
        centerChance = centerchance;
        minSize = minsize;
        maxSize = maxsize;
    }
    GLint blockType, startHeight, endHeight, centerHeight, minSize, maxSize;
    GLfloat startChance, centerChance, endChance;
};

struct BillboardVertex
{
    glm::vec3 pos;
    glm::vec2 uvMult;
    GLubyte texUnit;
    GLubyte texID;
    GLubyte light[2];
    GLubyte color[4];
    GLubyte size;
    GLubyte xMod;
    GLubyte padding[2]; //needs to be 4 byte aligned
};

struct PhysicsBlockPosLight
{
    f32v3 pos; //12
    ColorRGB8 color; //15
    GLubyte pad1; //16
    ColorRGB8 overlayColor; //19
    GLubyte pad2; //20
    GLubyte light[2]; //22
    GLubyte pad3[2]; //24
};

struct TreeVertex
{
    glm::vec2 pos; //8
    glm::vec3 center; //20
    GLubyte lr, lg, lb, size; //24
    GLubyte tr, tg, tb, ltex; //28
};

//No idea how this works. Something to do with prime numbers, but returns # between -1 and 1
inline double PseudoRand(int x, int z)
{
     int n= (x & 0xFFFF) + ((z & 0x7FFF) << 16);
     n=(n<<13)^n;
     int nn=(n*(n*n*60493+z*19990303)+x*1376312589)&0x7fffffff;
     return 1.0-((double)nn/1073741824.0);
}


inline double PseudoRand(int n)
{
    n = (n << 13) ^ n;
    int nn = (n*(n*n * 60493 + n * 19990303) + n * 1376312589) & 0x7fffffff;
    return 1.0 - ((double)nn / 1073741824.0);
}

