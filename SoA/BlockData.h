#pragma once
#include "stdafx.h"

#include <SDL/SDL.h>

#include "BlockTextureMethods.h"
#include "CAEngine.h"
#include "ChunkMesh.h"
#include "Constants.h"
#include "Keg.h"
#include "Rendering.h"
#include "global.h"

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
#define GETBLOCKID(a) (((a) & 0x0FFF))
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

enum class BlockOcclusion {
    NONE,
    ALL,
    SELF,
    SELF_ONLY
};
KEG_ENUM_DECL(BlockOcclusion);

class BlockTextureLayer {
public:
    // Set defaults in constructor for no .tex file
    BlockTextureLayer() : 
        method(ConnectedTextureMethods::NONE),
        size(1),
        symmetry(ConnectedTextureSymmetry::NONE),
        reducedMethod(ConnectedTextureReducedMethod::NONE),
        useMapColor(""),
        colorMapIndex(0),
        floraHeight(0),
        totalWeight(0),
        numTiles(1),
        textureIndex(0),
        innerSeams(false),
        transparency(false),
        path(""),
        blockTextureFunc(BlockTextureMethods::getDefaultTextureIndex) {
        // Empty
    }

    static ui32 getFloraRows(ui32 floraMaxHeight) {
        return (floraMaxHeight * floraMaxHeight + floraMaxHeight) / 2;
    }

    // Sets the texture funct based on the method
    // needs to have the method
    void initBlockTextureFunc() {
        switch (method) {
            case ConnectedTextureMethods::CONNECTED:
                blockTextureFunc = BlockTextureMethods::getConnectedTextureIndex;
                break;
            case ConnectedTextureMethods::RANDOM:
                blockTextureFunc = BlockTextureMethods::getRandomTextureIndex;
                break;
            case ConnectedTextureMethods::GRASS:
                blockTextureFunc = BlockTextureMethods::getGrassTextureIndex;
                break;
            case ConnectedTextureMethods::HORIZONTAL:
                blockTextureFunc = BlockTextureMethods::getHorizontalTextureIndex;
                break;
            case ConnectedTextureMethods::VERTICAL:
                blockTextureFunc = BlockTextureMethods::getVerticalTextureIndex;
                break;
            case ConnectedTextureMethods::FLORA:
                blockTextureFunc = BlockTextureMethods::getFloraTextureIndex;
                break;
            default:
                break;
        }
    }

    i32 getBlockTextureIndex(BlockTextureMethodParams& params, ColorRGB8 color) const {
        i32 index = textureIndex;
        params.set(this, color);
        blockTextureFunc(params, index);
        return index;
    }

    ConnectedTextureMethods method;
    i32v2 size;
    ConnectedTextureSymmetry symmetry;
    ConnectedTextureReducedMethod reducedMethod;
    nString useMapColor;
    ui32 colorMapIndex;
    ui32 floraHeight;
    Array<i32> weights;
    i32 totalWeight;
    i32 numTiles;
    i32 textureIndex;
    bool innerSeams;
    bool transparency;
    nString path;
    BlockTextureFunc blockTextureFunc;

    /// "less than" operator for inserting into sets in TexturePackLoader
    bool operator<(const BlockTextureLayer& b) const;
};
KEG_TYPE_DECL(BlockTextureLayer);

struct BlockTexture {
    BlockTexture() : blendMode(BlendType::ALPHA){};
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

struct BlockTextureFaces {
public:
    union {
        ui32 array[6];       ///  Access 6-sided block textures as an array
        struct {
            ui32 px;  /// Positive x-axis texture
            ui32 py;  /// Positive y-axis texture
            ui32 pz;  /// Positive z-axis texture
            ui32 nx;  /// Negative x-axis texture
            ui32 ny;  /// Negative y-axis texture
            ui32 nz;  /// Negative z-axis texture
        }; /// Textures named in cardinal convention
    };
    
    ui32& operator[] (const i32& i) {
        return array[i];
    }
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
    ui16 lightColorPacked; /// 5 bit RGB light color packed into a ui16
    i16 waterMeshLevel;
    i16 floatingAction;
    ui16 spawnerVal;
    ui16 sinkVal;
    ui16 explosionRays;
    ui16 floraHeight = 0;
    ui16 liquidStartID = 0;
    ui16 liquidLevels = 0;

    BlockOcclusion occlude;

    MeshType meshType;

    GLfloat moveMod;
    GLfloat explosionResistance;
    GLfloat explosivePower;
    GLfloat flammability;
    GLfloat explosionPowerLoss;
    f32v3 colorFilter;

    int caIndex = -1;
    CA_ALGORITHM caAlg = CA_ALGORITHM::NONE;
    nString caFilePath = "";

    ColorRGB8 color;
    ColorRGB8 overlayColor;
    ColorRGB8 averageColor;
    ColorRGB8 lightColor;
    ui8 particleTex;
    bool powderMove;
    bool collide;
    bool waterBreak;
    bool blockLight;
    bool useable;
    bool allowLight;
    bool isCrushable;
    bool isSupportive;
    bool active;

    BlockTexture pxTexInfo, pyTexInfo, pzTexInfo, nxTexInfo, nyTexInfo, nzTexInfo;
    // BEGIN TEXTURES - DONT CHANGE THE ORDER: Used BY ChunkMesher for connected textures
    BlockTextureFaces base;
    BlockTextureFaces overlay;
    BlockTextureFaces normal;
    // END

    nString leftTexName, rightTexName, frontTexName, backTexName, topTexName, bottomTexName, particleTexName;
    nString name, emitterName, emitterOnBreakName, emitterRandomName;
    class ParticleEmitter *emitter, *emitterOnBreak, *emitterRandom;

    std::vector <ColorRGB8> altColors;
    std::vector <ItemDrop> itemDrops;
};
KEG_TYPE_DECL(Block);

void SetBlockAvgTexColors();

void DrawHeadBlock(glm::dvec3 position, glm::mat4 &VP, Block *block, int flags, float light, float sunlight);