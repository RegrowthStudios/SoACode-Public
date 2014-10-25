#pragma once
#include "stdafx.h"

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
    NONE,
    CONNECTED,
    HORIZONTAL,
    VERTICAL,
    GRASS,
    REPEAT,
    RANDOM,
    FLORA
};
KEG_ENUM_DECL(ConnectedTextureMethods);

enum class ConnectedTextureSymmetry {
    NONE,
    OPPOSITE,
    ALL
};
KEG_ENUM_DECL(ConnectedTextureSymmetry);

enum class ConnectedTextureReducedMethod {
    NONE,
    TOP,
    BOTTOM
};
KEG_ENUM_DECL(ConnectedTextureReducedMethod);

struct BlockTextureLayer {
    // Set defaults in constructor for no .tex file
    BlockTextureLayer() : 
        method(ConnectedTextureMethods::NONE),
        size(1),
        symmetry(ConnectedTextureSymmetry::NONE),
        reducedMethod(ConnectedTextureReducedMethod::NONE),
        useMapColor(""),
        colorMapIndex(0),
        totalWeight(0),
        numTiles(1),
        textureIndex(0),
        innerSeams(false),
        transparency(false),
        path("") {
        // Empty
    }
    ConnectedTextureMethods method;
    i32v2 size;
    ConnectedTextureSymmetry symmetry;
    ConnectedTextureReducedMethod reducedMethod;
    nString useMapColor;
    ui32 colorMapIndex;
    Array<i32> weights;
    i32 totalWeight;
    i32 numTiles;
    i32 textureIndex;
    bool innerSeams;
    bool transparency;
    nString path;

    /// "less than" operator for inserting into sets in TexturePackLoader
    bool operator<(const BlockTextureLayer& b) const;
};
KEG_TYPE_DECL(BlockTextureLayer);

struct BlockTexture {
    BlockTexture() : blendMode(BlendType::REPLACE){};
    BlockTexture(const BlockTextureLayer& b, const BlockTextureLayer& o, BlendType bt) :
        base(b), overlay(o), blendMode(bt){
        // Empty
    }
    BlockTextureLayer base;
    BlockTextureLayer overlay;

    BlendType blendMode;
};
KEG_TYPE_DECL(BlockTexture);

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
    void GetBlockColor(ColorRGB8& baseColor, ColorRGB8& overlayColor, GLuint flags, int temperature, int rainfall, const BlockTexture& blockTexture);
    void GetBlockColor(ColorRGB8& baseColor, GLuint flags, int temperature, int rainfall, const BlockTexture& blockTexture);

    void SetAvgTexColors();

    ui16 ID;
    ui16 burnTransformID;
    i16 waveEffect;
    ui16 lightColor;
    i16 physicsProperty;
    i16 waterMeshLevel;
    i16 floatingAction;
    i16 occlude;
    ui16 spawnerVal;
    ui16 sinkVal;
    ui16 explosionRays;

    MeshType meshType;

    GLfloat moveMod;
    GLfloat explosionResistance;
    GLfloat explosivePower;
    GLfloat flammability;
    GLfloat powerLoss;
    f32v3 colorFilter;

    ColorRGB8 color;
    ColorRGB8 overlayColor;
    ColorRGB8 averageColor;
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

    std::vector <ColorRGB8> altColors;
    std::vector <ItemDrop> itemDrops;
};

void SetBlockAvgTexColors();

void DrawHeadBlock(glm::dvec3 position, glm::mat4 &VP, Block *block, int flags, float light, float sunlight);