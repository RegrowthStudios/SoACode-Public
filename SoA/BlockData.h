#pragma once
#include "stdafx.h"

#include <SDL/SDL.h>
#include <Vorb/io/Keg.h>
#include <Vorb/graphics/ImageIO.h>
#include <Vorb/voxel/VoxCommon.h>

#include "CAEngine.h"
#include "ChunkMesh.h"
#include "BlockTexture.h"
#include "Constants.h"
#include "Rendering.h"
#include "Item.h"

#define GETFLAGS(a) ((a) >> 12)
#define GETFLAG1(a) (((a) & 0x8000) >> 15)
#define GETFLAG2(a) (((a) & 0x4000) >> 14)
#define GETFLAG3(a) (((a) & 0x2000) >> 13)
#define GETFLAG4(a) (((a) & 0x1000) >> 12)
#define GETBLOCKID(a) (((a) & 0x0FFF))
#define SETFLAGS(a, b) ((a) = ((a) | ((b) << 12)))

enum class BlockOcclusion {
    NONE,
    ALL,
    SELF,
    SELF_ONLY
};
KEG_ENUM_DECL(BlockOcclusion);

enum BlockMaterials { M_NONE, M_STONE, M_MINERAL };

//TODO(Ben): Crush this bullshit
class BlockVariable
{
public:
    //VarType 0 = int, 1 = float
    BlockVariable(){}
    BlockVariable(float Min, float Max, float Step, int ByteOffset, int VarType){ min = Min; max = Max; step = Step; byteOffset = ByteOffset; varType = VarType; controlType = 0; editorAccessible = 1;}
    float min, max;
    float step;
    int byteOffset;
    int varType; //0 = int, 1 = float, 2 = byte
    int controlType; //0 = slider, 1 = list box
    bool editorAccessible;
    std::vector<nString> listNames;
};

class ItemDrop
{
    ItemDrop(Item *itm, int Num){
        item = itm;
        num = Num;
    }
    Item *item;
    int num;
};

class BlockTextureFaces {
public:
    union {
        ui32 array[6]; /// Access 6-sided block textures as an array
        class {
        public:
            ui32 nx;  /// Negative x-axis texture
            ui32 px;  /// Positive x-axis texture
            ui32 ny;  /// Negative y-axis texture
            ui32 py;  /// Positive y-axis texture
            ui32 nz;  /// Negative z-axis texture
            ui32 pz;  /// Positive z-axis texture
        }; /// Textures named in cardinal convention
    };
    
    ui32& operator[] (const i32& i) {
        return array[i];
    }
};

typedef nString BlockIdentifier; ///< Unique identifier key for blocks

class Block
{
public:
    Block();

    void GetBlockColor(ColorRGB8& baseColor, ColorRGB8& overlayColor, GLuint flags, int temperature, int rainfall, const BlockTexture* blockTexture);
    void GetBlockColor(ColorRGB8& baseColor, GLuint flags, int temperature, int rainfall, const BlockTexture* blockTexture);

    void SetAvgTexColors();

    BlockIdentifier name;
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

    BlockTexture* textures[6];
    nString texturePaths[6];
   
    // TODO(BEN): a bit redudant isn't it?
    // BEGIN TEXTURES - DONT CHANGE THE ORDER: Used BY ChunkMesher for connected textures
    BlockTextureFaces base;
    BlockTextureFaces overlay;
    BlockTextureFaces normal;
    // END

    // TODO(Ben): NOPE
    nString particleTexName;
    nString emitterName, emitterOnBreakName, emitterRandomName;
    class ParticleEmitter *emitter, *emitterOnBreak, *emitterRandom;

    std::vector <ColorRGB8> altColors;
    std::vector <ItemDrop> itemDrops;
};
KEG_TYPE_DECL(Block);

void SetBlockAvgTexColors();

void DrawHeadBlock(glm::dvec3 position, glm::mat4 &VP, Block *block, int flags, float light, float sunlight);