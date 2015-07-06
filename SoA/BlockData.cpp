#include "stdafx.h"
#include "BlockData.h"

#include "BlockPack.h"
#include "Errors.h"
#include "GameManager.h"
#include "SoaOptions.h"
#include "Rendering.h"
#include "ZipFile.h"

KEG_ENUM_DEF(BlockOcclusion, BlockOcclusion, e) {
    e.addValue("none", BlockOcclusion::NONE);
    e.addValue("self", BlockOcclusion::SELF);
    e.addValue("selfOnly", BlockOcclusion::SELF_ONLY);
    e.addValue("all", BlockOcclusion::ALL);
}

KEG_TYPE_DEF_SAME_NAME(Block, kt) {
    kt.addValue("ID", keg::Value::basic(offsetof(Block, temp), keg::BasicType::I32));
    kt.addValue("name", keg::Value::basic(offsetof(Block, name), keg::BasicType::STRING));
    kt.addValue("burnTransformID", keg::Value::basic(offsetof(Block, burnTransformID), keg::BasicType::UI16));
    kt.addValue("waveEffect", keg::Value::basic(offsetof(Block, waveEffect), keg::BasicType::I16));
    kt.addValue("lightColor", keg::Value::basic(offsetof(Block, lightColor), keg::BasicType::UI8_V3));
    kt.addValue("caPhysics", keg::Value::basic(offsetof(Block, caFilePath), keg::BasicType::STRING));
    kt.addValue("waterMeshLevel", keg::Value::basic(offsetof(Block, waterMeshLevel), keg::BasicType::I16));
    kt.addValue("floatingAction", keg::Value::basic(offsetof(Block, floatingAction), keg::BasicType::I16));
    kt.addValue("occlusion", keg::Value::custom(offsetof(Block, occlude), "BlockOcclusion", true));
    kt.addValue("spawnerVal", keg::Value::basic(offsetof(Block, spawnerVal), keg::BasicType::UI16));
    kt.addValue("sinkVal", keg::Value::basic(offsetof(Block, sinkVal), keg::BasicType::UI16));
    kt.addValue("explosionRays", keg::Value::basic(offsetof(Block, explosionRays), keg::BasicType::UI16));
    kt.addValue("meshType", keg::Value::custom(offsetof(Block, meshType), "MeshType", true));
    kt.addValue("moveMod", keg::Value::basic(offsetof(Block, moveMod), keg::BasicType::F32));
    kt.addValue("explosionResistance", keg::Value::basic(offsetof(Block, explosionResistance), keg::BasicType::F32));
    kt.addValue("explosionPower", keg::Value::basic(offsetof(Block, explosivePower), keg::BasicType::F32));
    kt.addValue("flammability", keg::Value::basic(offsetof(Block, flammability), keg::BasicType::F32));
    kt.addValue("explosionPowerLoss", keg::Value::basic(offsetof(Block, explosionPowerLoss), keg::BasicType::F32));
    kt.addValue("lightColorFilter", keg::Value::basic(offsetof(Block, colorFilter), keg::BasicType::F32_V3));
    kt.addValue("emitter", keg::Value::basic(offsetof(Block, emitterName), keg::BasicType::STRING));
    kt.addValue("movesPowder", keg::Value::basic(offsetof(Block, powderMove), keg::BasicType::BOOL));
    kt.addValue("collide", keg::Value::basic(offsetof(Block, collide), keg::BasicType::BOOL));
    kt.addValue("waterBreak", keg::Value::basic(offsetof(Block, waterBreak), keg::BasicType::BOOL));
    kt.addValue("scatterSunRays", keg::Value::basic(offsetof(Block, blockLight), keg::BasicType::BOOL));
    kt.addValue("useable", keg::Value::basic(offsetof(Block, useable), keg::BasicType::BOOL));
    kt.addValue("allowsLight", keg::Value::basic(offsetof(Block, allowLight), keg::BasicType::BOOL));
    kt.addValue("crushable", keg::Value::basic(offsetof(Block, isCrushable), keg::BasicType::BOOL));
    kt.addValue("supportive", keg::Value::basic(offsetof(Block, isSupportive), keg::BasicType::BOOL));
}

/// "less than" operator for inserting into sets in TexturePackLoader
bool BlockTextureLayer::operator<(const BlockTextureLayer& b) const {

    // Helper macro for checking if !=
#define LCHECK(a) if (a < b.##a) { return true; } else if (a > b.##a) { return false; }

    LCHECK(path);
    LCHECK(method);
    LCHECK(size.x);
    LCHECK(size.y);
    LCHECK(symmetry);
    LCHECK(reducedMethod);
    LCHECK(useMapColor);
    LCHECK(weights.size());
    LCHECK(totalWeight);
    LCHECK(numTiles);
    LCHECK(innerSeams);
    LCHECK(transparency);
    return false;
}

// TODO(Ben): LOL
Block::Block() : emitterName(""), 
emitterOnBreakName(""), 
emitter(nullptr),
emitterOnBreak(nullptr),
emitterRandom(nullptr),
emitterRandomName(""),
lightColor(0, 0, 0) {
    allowLight = false;
    ID = 0;
    name = particleTexName = "";
    memset(textures, 0, sizeof(textures));
    particleTex = 0;
    collide = true;
    occlude = BlockOcclusion::ALL;
    meshType = MeshType::BLOCK;
    waveEffect = 0;
    explosionResistance = 1.0;
    active = 0;
    useable = 1;
    blockLight = true;
    waterMeshLevel = 0;
    waterBreak = false;
    isCrushable = false;
    floatingAction = 1;
    flammability = 0.0f;
    isSupportive = true;
    explosivePower = 0.0;
    explosionPowerLoss = 0.0;
    explosionRays = 0;
    powderMove = true;
    moveMod = 1.0f;
    spawnerVal = 0;
    sinkVal = 0;
    colorFilter = f32v3(1.0f);
}

void Block::getBlockColor(color3& baseColor, color3& overlayColor, GLuint flags, int temperature, int rainfall, const BlockTexture* blockTexture) const
{
    int index = (255 - rainfall) * 256 + temperature;
    //base color
    // TODO(Ben): Handle this
    /*if (blockTexture.base.colorMap) {
        ui8v3* bytes = blockTexture.base.colorMap->bytesUI8v3 + index;
        //Average the map color with the base color
        baseColor.r = (ui8)(((float)Block::color.r * (float)bytes->r) / 255.0f);
        baseColor.g = (ui8)(((float)Block::color.g * (float)bytes->g) / 255.0f);
        baseColor.b = (ui8)(((float)Block::color.b * (float)bytes->b) / 255.0f);
    } else */if (altColors.size() >= flags && flags){ //alt colors, for leaves and such
        baseColor = altColors[flags - 1];
    } else{
        baseColor = color;
    }
    //overlay color
    /*if (blockTexture.overlay.colorMap) {
        ui8v3* bytes = blockTexture.overlay.colorMap->bytesUI8v3 + index;
        //Average the map color with the base color
        overlayColor.r = (ui8)(((float)Block::overlayColor.r * (float)bytes->r) / 255.0f);
        overlayColor.g = (ui8)(((float)Block::overlayColor.g * (float)bytes->g) / 255.0f);
        overlayColor.b = (ui8)(((float)Block::overlayColor.b * (float)bytes->b) / 255.0f);
    } else */if (altColors.size() >= flags && flags){ //alt colors, for leaves and such
        overlayColor= altColors[flags - 1];
    } else{
        overlayColor = Block::overlayColor;
    }
}

void Block::getBlockColor(color3& baseColor, GLuint flags, int temperature, int rainfall, const BlockTexture* blockTexture) const
{
    int index = (255 - rainfall) * 256 + temperature;
    //base color
    /*if (blockTexture.base.colorMap) {
        ui8v3* bytes = blockTexture.base.colorMap->bytesUI8v3 + index;
        //Average the map color with the base color
        baseColor.r = (ui8)(((float)Block::color.r * (float)bytes->r) / 255.0f);
        baseColor.g = (ui8)(((float)Block::color.g * (float)bytes->g) / 255.0f);
        baseColor.b = (ui8)(((float)Block::color.b * (float)bytes->b) / 255.0f);
    } else*/ if (altColors.size() >= flags && flags){ //alt colors, for leaves and such
        baseColor = altColors[flags - 1];
    } else{
        baseColor = color;
    }
}

//TODO: Ben re-implement this
void Block::SetAvgTexColors()
{
    /*int bindex;
    size_t texUnit = pxTex / 256;
    if (texUnit < blockPacks.size()){
    bindex = pxTex%256;
    aTexr = blockPacks[texUnit].avgColors[bindex][0];
    aTexg = blockPacks[texUnit].avgColors[bindex][1];
    aTexb = blockPacks[texUnit].avgColors[bindex][2];

    aTexr = GLubyte((((float)aTexr)*((float)tr))/255.0f);
    aTexg = GLubyte((((float)aTexg)*((float)tg))/255.0f);
    aTexb = GLubyte((((float)aTexb)*((float)tb))/255.0f);
    }*/
}

void SetBlockAvgTexColors()
{
    /*for (size_t i = 0; i < Blocks.size(); i++){
        if (Blocks[i].active){
            Blocks[i].SetAvgTexColors();
        }
    }*/
}

void DrawHeadBlock(glm::dvec3 position, glm::mat4 &VP, Block *block, int flags, float light, float sunlight)
{
  
}

ui16 idLowWater = 0;
ui16& getLowWaterID() {
    return idLowWater;
}
ui16 idVisitedNode = 0;
ui16& getVisitedNodeID() {
    return idVisitedNode;
}
