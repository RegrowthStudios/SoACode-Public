#pragma once
#include <Windows.h>

#include <SDL/SDL.h>

#include "Constants.h"
#include "global.h"
#include "Keg.h"
#include "Rendering.h"
#include "ChunkMesh.h"

#define VISITED_NODE 3945
#define LOWWATER 3946
#define FULLWATER 4045
#define FULLPRESSURE 4095
#define WATERSOURCE 3946

#define GETFLAGS(a) ((a) >> 12)
#define GETFLAG1(a) (((a) & 0x8000) >> 15)
#define GETFLAG2(a) (((a) & 0x4000) >> 14)
#define GETFLAG3(a) (((a) & 0x2000) >> 13)
#define GETFLAG4(a) (((a) & 0x1000) >> 12)
#define GETBLOCKTYPE(a) (((a) & 0x0FFF))
#define GETBLOCK(a) ((Blocks[((a) & 0x0FFF)]))
#define SETFLAGS(a, b) ((a) = ((a) | ((b) << 12)))

enum class ConnectedTextureMethods {
    CTM_NONE,
    CTM_CONNECTED,
    CTM_HORIZONTAL,
    CTM_VERTICAL,
    CTM_GRASS,
    CTM_REPEAT,
    CTM_RANDOM
};
KEG_ENUM_DECL(ConnectedTextureMethods);

enum class ConnectedTextureSymmetry {
    SYMMETRY_NONE,
    SYMMETRY_OPPOSITE,
    SYMMETRY_ALL
};
KEG_ENUM_DECL(ConnectedTextureSymmetry);


struct BlockTextureLayer {
    ConnectedTextureMethods method;
    i32v2 size;
    ConnectedTextureSymmetry symmetry;
    nString useMapColor;
    Array<i32> weights;
    i32 totalWeight;
    i32 numTiles;
    i32 textureIndex;
    bool innerSeams;
    bool transparency;
    nString path;
};
KEG_TYPE_DECL(BlockTextureLayer);

struct BlockTexture {
    BlockTextureLayer base;
    BlockTextureLayer overlay;

    BlendType blendMode;
};
KEG_TYPE_DECL(BlockTexture);

//struct BlockTexture {
//    i32 method;
//    i32 overlayMethod;
//
//    i32 width;
//    i32 overlayWidth;
//
//    i32 height;
//    i32 overlayHeight;
//
//    i32 symmetry;
//    i32 overlaySymmetry;
//
//    bool innerSeams;
//    bool overlayInnerSeams;
//
//    i32 useMapColor;
//    i32 overlayUseMapColor;
//
//    std::vector<i32> weights;
//    std::vector<i32> overlayWeights;
//
//    i32 totalWeight;
//    i32 overlayTotalWeight;
//
//    i32 numTiles;
//    i32 overlayNumTiles;
//
//    i32 textureIndex;
//    i32 overlayTextureIndex;
//
//    i32 blendMode;
//
//    nString basePath;
//    nString overlayPath;
//};
//KEG_TYPE_DECL(BlockTexture);

using namespace std;


extern vector <int> TextureUnitIndices;

const int numBlocks = 4096;

extern vector <class Block> Blocks;

extern vector <int> TextureUnitIndices;

//TODO: KILL ALL OF THIS CRAP

enum Blocks { NONE, DIRT, DIRTGRASS, STONE, WOOD, SAND, SNOW = 12, ICE = 13, REDSAND = 21, FIRE = 1000};

enum Blocks2 { WHITEFLOWERS = 256, TALLGRASS, TALLWEED, YELLOWGRASS, DRYGRASS, FLOWERGRASS, TALLLIGHTGRASS, SHORTLIGHTGRASS, REDBUSH, SHORTDARKGRASS, JUNGLEPLANT1, BLUEFLOWERS, LARGEREDFLOWER, PURPLEFLOWER, HEARTFLOWER, DANDILION };

enum SmallMushrooms{ SMALLCYANMUSHROOM = 272, SMALLMAGENTAMUSHROOM };

enum LeafBlocks { LEAVES1 = 288, LEAVES2};

enum LeafBlocksTextures { T_LEAVES1 = 288, T_LEAVES2 };

enum BlockLights { LIGHT1 = 16, LIGHT2, LIGHT3 };

enum BlocksMinerals { COAL = 32, IRON, GOLD, SILVER, COPPER, TIN, LEAD, PLATINUM, DOLOMITE, DIAMOND, RUBY, EMERALD, SAPPHIRE, BAUXITE, MAGNETITE, MALACHITE, 
                        EMBERNITE, SULFUR, CHROALLON, SEAMORPHITE, THORNMITE, MORPHIUM, OXYGENIUM, SUNANITE, CAMONITE, SUBMARIUM, TRITANIUM, URANIUM, TUNGSTEN};
enum BlocksMinerals2 { BLUECRYSTAL = 80 };

enum BlocksMushroom { BLUEMUSHROOMBLOCK = 96, PURPLEMUSHROOMBLOCK, LAMELLABLOCK, MUSHROOMSTEM, GREYMUSHROOMBLOCK, DARKPURPLEMUSHROOMBLOCK, DARKBLUEMUSHROOMBLOCK};

enum BlocksStones { SANDSTONE = 64, SHALE, LIMESTONE, GRAVEL, BASALT, SLATE, GNEISS, GRANITE, MARBLE, REDSANDSTONE = 75};

enum BlockTextures1{ T_DIRT, T_DIRTGRASS, T_GRASS, T_STONE , T_WATER, T_SAND, T_WOOD, T_SNOW = 12, T_ICE = 13, T_REDSAND = 21};

const int physStart = 2;
enum PhysicsProperties {P_NONE, P_SOLID, P_LIQUID, P_POWDER, P_SNOW};
enum BlockMaterials { M_NONE, M_STONE, M_MINERAL };

enum Explosives { TNT = 112, NITRO, C4 };
enum ExplosivesTextures {T_TNT = 112, T_NITRO = 115, T_C4 = 118};

enum BuildingBlocks { GLASS = 160, CONCRETE, BRICKS, PLANKS, COBBLE };
enum TransparentBuildingBlocksTextures{ T_GLASS = 304,};
enum BuildingBlocksTextures { T_CONCRETE = 161, T_BRICKS, T_PLANKS, T_COBBLE };

//struct BlockTexture {
//    BlockTexture() : method(0), width(1), height(1), innerSeams(false), symmetry(0), totalWeight(0), numTiles(0), basePath(""), useMapColor(0), overlayUseMapColor(0),
//        overlayMethod(0), overlayWidth(1), overlayHeight(1), overlayInnerSeams(false), overlaySymmetry(0), overlayTotalWeight(0), overlayNumTiles(0),
//        blendMode(0), textureIndex(0), overlayTextureIndex(1), overlayPath(""){}
//
//    int method;
//    int overlayMethod;
//
//    int width;
//    int overlayWidth;
//
//    int height;
//    int overlayHeight;
//
//    int symmetry;
//    int overlaySymmetry;
//
//    bool innerSeams;
//    bool overlayInnerSeams;
//
//    int useMapColor;
//    int overlayUseMapColor;
//
//    vector <int> weights;
//    vector <int> overlayWeights;
//
//    int totalWeight;
//    int overlayTotalWeight;
//
//    int numTiles;
//    int overlayNumTiles;
//   
//    int textureIndex;
//    int overlayTextureIndex;
//   
//    int blendMode;
//    
//    string basePath;
//    string overlayPath;
//};

extern int connectedTextureOffsets[256];
extern int grassTextureOffsets[32];

void initConnectedTextures();

struct BlockVariable
{
    //VarType 0 = int, 1 = float
    BlockVariable(){}
    BlockVariable(float Min, float Max, float Step, int ByteOffset, int VarType){ min = Min; max = Max; step = Step; byteOffset = ByteOffset; varType = VarType; controlType = 0; editorAccessible = 1;}
    float min, max;
    float step;
    int byteOffset;
    int varType; //0 = int, 1 = float, 2 = byte
    int controlType; //0 = slider, 1 = list box
    bool editorAccessible;
    vector <string> listNames;
};

extern map <string, BlockVariable> blockVariableMap;

struct ItemDrop
{
    ItemDrop(Item *itm, int Num){
        item = itm;
        num = Num;
    }
    Item *item;
    int num;
};

class Block
{
public:
    Block();

    void InitializeTexture();
    void GetBlockColor(GLubyte baseColor[3], GLubyte overlayColor[3], GLuint flags, int temperature, int rainfall, const BlockTexture& blockTexture);
    void GetBlockColor(GLubyte baseColor[3], GLuint flags, int temperature, int rainfall, const BlockTexture& blockTexture);

    void SetAvgTexColors();

    ui16 ID;
    ui16 burnTransformID;
    i16 waveEffect;
    i16 health;
    ui16 lightColor;
    i16 physicsProperty;
    i16 material;
    i16 waterMeshLevel;
    i16 floatingAction;
    i16 occlude;
    ui16 spawnerVal;
    ui16 sinkVal;
    ui16 explosionRays;

    MeshType meshType;

    GLfloat moveMod;
    GLfloat value;
    GLfloat weight;
    GLfloat explosionResistance;
    GLfloat explosivePower;
    GLfloat flammability;
    GLfloat powerLoss; 

    ui8 color[3];
    ui8 overlayColor[3];
    ui8 averageColor[3];
    ui8 particleTex;
    ui8 powderMove;
    ui8 collide;
    ui8 waterBreak;
    ui8 isLight;
    ui8 blockLight;
    ui8 useable;
    ui8 allowLight;
    ui8 isCrushable;
    ui8 isSupportive;
    ui8 active;

    BlockTexture pxTexInfo, pyTexInfo, pzTexInfo, nxTexInfo, nyTexInfo, nzTexInfo;
    // BEGIN TEXTURES - DONT CHANGE THE ORDER: Used BY ChunkMesher for connected textures
    int pxTex, pyTex, pzTex, nxTex, nyTex, nzTex;
    int pxOvTex, pyOvTex, pzOvTex, nxOvTex, nyOvTex, nzOvTex;
    // END

    // normal maps
    int pxNMap, pyNMap, pzNMap, nxNMap, nyNMap, nzNMap;

    string leftTexName, rightTexName, frontTexName, backTexName, topTexName, bottomTexName, particleTexName;
    string name, emitterName, emitterOnBreakName, emitterRandomName;
    class ParticleEmitter *emitter, *emitterOnBreak, *emitterRandom;

    std::vector <glm::ivec3> altColors;
    std::vector <ItemDrop> itemDrops;
};

void SetBlockAvgTexColors();

void DrawHeadBlock(glm::dvec3 position, glm::mat4 &VP, Block *block, int flags, float light, float sunlight);