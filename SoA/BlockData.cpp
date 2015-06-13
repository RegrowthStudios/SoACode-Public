#include "stdafx.h"
#include "BlockData.h"

#include "BlockPack.h"
#include "Errors.h"
#include "GameManager.h"
#include "SoaOptions.h"
#include "Rendering.h"
#include "TerrainGenerator.h"
#include "TexturePackLoader.h"
#include "ZipFile.h"

KEG_ENUM_DEF(MeshType, MeshType, e) {
    e.addValue("none", MeshType::NONE);
    e.addValue("cube", MeshType::BLOCK);
    e.addValue("leaves", MeshType::LEAVES);
    e.addValue("triangle", MeshType::FLORA);
    e.addValue("cross", MeshType::CROSSFLORA);
    e.addValue("liquid", MeshType::LIQUID);
    e.addValue("flat", MeshType::FLAT);
}
KEG_ENUM_DEF(ConnectedTextureMethods, ConnectedTextureMethods, e) {
    e.addValue("none", ConnectedTextureMethods::NONE);
    e.addValue("connect", ConnectedTextureMethods::CONNECTED);
    e.addValue("random", ConnectedTextureMethods::RANDOM);
    e.addValue("repeat", ConnectedTextureMethods::REPEAT);
    e.addValue("grass", ConnectedTextureMethods::GRASS);
    e.addValue("horizontal", ConnectedTextureMethods::HORIZONTAL);
    e.addValue("vertical", ConnectedTextureMethods::VERTICAL);
    e.addValue("flora", ConnectedTextureMethods::FLORA);
}
KEG_ENUM_DEF(ConnectedTextureSymmetry, ConnectedTextureSymmetry, e) {
    e.addValue("none", ConnectedTextureSymmetry::NONE);
    e.addValue("opposite", ConnectedTextureSymmetry::OPPOSITE);
    e.addValue("all", ConnectedTextureSymmetry::ALL);
}
KEG_ENUM_DEF(BlockOcclusion, BlockOcclusion, e) {
    e.addValue("none", BlockOcclusion::NONE);
    e.addValue("self", BlockOcclusion::SELF);
    e.addValue("selfOnly", BlockOcclusion::SELF_ONLY);
    e.addValue("all", BlockOcclusion::ALL);
}
KEG_ENUM_DEF(ConnectedTextureReducedMethod, ConnectedTextureReducedMethod, e) {
    e.addValue("none", ConnectedTextureReducedMethod::NONE);
    e.addValue("top", ConnectedTextureReducedMethod::TOP);
    e.addValue("bottom", ConnectedTextureReducedMethod::BOTTOM);
}
KEG_ENUM_DEF(BlendType, BlendType, e) {
    e.addValue("add", BlendType::ADD);
    e.addValue("multiply", BlendType::MULTIPLY);
    e.addValue("replace", BlendType::ALPHA);
    e.addValue("subtract", BlendType::SUBTRACT);
}

KEG_TYPE_DEF_SAME_NAME(BlockTextureLayer, kt) {
      kt.addValue("method", keg::Value::custom(offsetof(BlockTextureLayer, method), "ConnectedTextureMethods", true));
      kt.addValue("reducedMethod", keg::Value::custom(offsetof(BlockTextureLayer, reducedMethod), "ConnectedTextureReducedMethod", true));
      KEG_TYPE_INIT_ADD_MEMBER(kt, BlockTextureLayer, size, I32_V2);
      kt.addValue("symmetry", keg::Value::custom(offsetof(BlockTextureLayer, symmetry), "ConnectedTextureSymmetry", true));
      KEG_TYPE_INIT_ADD_MEMBER(kt, BlockTextureLayer, innerSeams, BOOL);
      KEG_TYPE_INIT_ADD_MEMBER(kt, BlockTextureLayer, transparency, BOOL);
      kt.addValue("height", keg::Value::basic(offsetof(BlockTextureLayer, floraHeight), keg::BasicType::UI32));
      KEG_TYPE_INIT_ADD_MEMBER(kt, BlockTextureLayer, useMapColor, STRING);
      KEG_TYPE_INIT_ADD_MEMBER(kt, BlockTextureLayer, totalWeight, I32);
      KEG_TYPE_INIT_ADD_MEMBER(kt, BlockTextureLayer, numTiles, I32);
      kt.addValue("weights", keg::Value::array(offsetof(BlockTextureLayer, weights), keg::BasicType::I32));
      KEG_TYPE_INIT_ADD_MEMBER(kt, BlockTextureLayer, textureIndex, I32);
      KEG_TYPE_INIT_ADD_MEMBER(kt, BlockTextureLayer, path, STRING);
}

KEG_TYPE_DEF_SAME_NAME(BlockTexture, kt) {
    kt.addValue("base", keg::Value::custom(offsetof(BlockTexture, base), "BlockTextureLayer"));
    kt.addValue("overlay", keg::Value::custom(offsetof(BlockTexture, overlay), "BlockTextureLayer"));
    kt.addValue("blendMode", keg::Value::custom(offsetof(BlockTexture, blendMode), "BlendType", true));
}

KEG_TYPE_DEF_SAME_NAME(Block, kt) {
    kt.addValue("ID", keg::Value::basic(offsetof(Block, ID), keg::BasicType::UI16));
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
    kt.addValue("color", keg::Value::basic(offsetof(Block, color), keg::BasicType::UI8_V3));
    kt.addValue("emitter", keg::Value::basic(offsetof(Block, emitterName), keg::BasicType::STRING));
    kt.addValue("movesPowder", keg::Value::basic(offsetof(Block, powderMove), keg::BasicType::BOOL));
    kt.addValue("collide", keg::Value::basic(offsetof(Block, collide), keg::BasicType::BOOL));
    kt.addValue("waterBreak", keg::Value::basic(offsetof(Block, waterBreak), keg::BasicType::BOOL));
    kt.addValue("scatterSunRays", keg::Value::basic(offsetof(Block, blockLight), keg::BasicType::BOOL));
    kt.addValue("useable", keg::Value::basic(offsetof(Block, useable), keg::BasicType::BOOL));
    kt.addValue("allowsLight", keg::Value::basic(offsetof(Block, allowLight), keg::BasicType::BOOL));
    kt.addValue("crushable", keg::Value::basic(offsetof(Block, isCrushable), keg::BasicType::BOOL));
    kt.addValue("supportive", keg::Value::basic(offsetof(Block, isSupportive), keg::BasicType::BOOL));
    kt.addValue("textureLeft", keg::Value::basic(offsetof(Block, leftTexName), keg::BasicType::STRING));
    kt.addValue("textureRight", keg::Value::basic(offsetof(Block, rightTexName), keg::BasicType::STRING));
    kt.addValue("textureFront", keg::Value::basic(offsetof(Block, frontTexName), keg::BasicType::STRING));
    kt.addValue("textureBack", keg::Value::basic(offsetof(Block, backTexName), keg::BasicType::STRING));
    kt.addValue("textureTop", keg::Value::basic(offsetof(Block, topTexName), keg::BasicType::STRING));
    kt.addValue("textureBottom", keg::Value::basic(offsetof(Block, bottomTexName), keg::BasicType::STRING));
}

std::vector <int> TextureUnitIndices;

int connectedTextureOffsets[256];
int grassTextureOffsets[32];

std::map <nString, BlockVariable> blockVariableMap;

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

Block::Block() : emitterName(""), 
emitterOnBreakName(""), 
emitter(nullptr),
emitterOnBreak(nullptr),
emitterRandom(nullptr),
emitterRandomName(""),
color(255, 255, 255),
overlayColor(255, 255, 255),
lightColor(0, 0, 0) {
    allowLight = false;
    ID = 0;
    name = leftTexName = rightTexName = backTexName = frontTexName = topTexName = bottomTexName = particleTexName = "";
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

void Block::GetBlockColor(ColorRGB8& baseColor, ColorRGB8& overlayColor, GLuint flags, int temperature, int rainfall, const BlockTexture& blockTexture)
{
    int index = (255 - rainfall) * 256 + temperature;
    //base color
    if (blockTexture.base.colorMap) {
        ui8v3* bytes = blockTexture.base.colorMap->bytesUI8v3 + index;
        //Average the map color with the base color
        baseColor.r = (ui8)(((float)Block::color.r * (float)bytes->r) / 255.0f);
        baseColor.g = (ui8)(((float)Block::color.g * (float)bytes->g) / 255.0f);
        baseColor.b = (ui8)(((float)Block::color.b * (float)bytes->b) / 255.0f);
    } else if (altColors.size() >= flags && flags){ //alt colors, for leaves and such
        baseColor = altColors[flags - 1];
    } else{
        baseColor = color;
    }
    //overlay color
    if (blockTexture.overlay.colorMap) {
        ui8v3* bytes = blockTexture.overlay.colorMap->bytesUI8v3 + index;
        //Average the map color with the base color
        overlayColor.r = (ui8)(((float)Block::overlayColor.r * (float)bytes->r) / 255.0f);
        overlayColor.g = (ui8)(((float)Block::overlayColor.g * (float)bytes->g) / 255.0f);
        overlayColor.b = (ui8)(((float)Block::overlayColor.b * (float)bytes->b) / 255.0f);
    } else if (altColors.size() >= flags && flags){ //alt colors, for leaves and such
        overlayColor= altColors[flags - 1];
    } else{
        overlayColor = Block::overlayColor;
    }
}

void Block::GetBlockColor(ColorRGB8& baseColor, GLuint flags, int temperature, int rainfall, const BlockTexture& blockTexture)
{
    int index = (255 - rainfall) * 256 + temperature;
    //base color
    if (blockTexture.base.colorMap) {
        ui8v3* bytes = blockTexture.base.colorMap->bytesUI8v3 + index;
        //Average the map color with the base color
        baseColor.r = (ui8)(((float)Block::color.r * (float)bytes->r) / 255.0f);
        baseColor.g = (ui8)(((float)Block::color.g * (float)bytes->g) / 255.0f);
        baseColor.b = (ui8)(((float)Block::color.b * (float)bytes->b) / 255.0f);
    } else if (altColors.size() >= flags && flags){ //alt colors, for leaves and such
        baseColor = altColors[flags - 1];
    } else{
        baseColor = color;
    }
}

void Block::InitializeTexture() {
    if (active) {
        // Default values for texture indices
        for (i32 i = 0; i < 6; i++) {
            base[i] = 0;
            normal[i] = -1;
            overlay[i] = -1;
        }

        GameManager::texturePackLoader->getBlockTexture(topTexName, textures[1]);
        base.py = textures[1].base.textureIndex;
        overlay.py = textures[1].overlay.textureIndex;

        GameManager::texturePackLoader->getBlockTexture(leftTexName, textures[3]);
        base.nx = textures[3].base.textureIndex;
        overlay.nx = textures[3].overlay.textureIndex;

        GameManager::texturePackLoader->getBlockTexture(rightTexName, textures[0]);
        base.px = textures[0].base.textureIndex;
        overlay.px = textures[0].overlay.textureIndex;

        GameManager::texturePackLoader->getBlockTexture(frontTexName, textures[2]);
        base.pz = textures[2].base.textureIndex;
        overlay.pz = textures[2].overlay.textureIndex;

        GameManager::texturePackLoader->getBlockTexture(backTexName, textures[5]);
        base.nz = textures[5].base.textureIndex;
        overlay.nz = textures[5].overlay.textureIndex;

        GameManager::texturePackLoader->getBlockTexture(bottomTexName, textures[4]);
        base.ny = textures[4].base.textureIndex;
        overlay.ny = textures[4].overlay.textureIndex;

        BlockTexture particleTexture;
        GameManager::texturePackLoader->getBlockTexture(particleTexName, particleTexture);
        particleTex = particleTexture.base.textureIndex;

        // Calculate flora height
        // TODO(Ben): Not really a good place for this
        if (textures[0].base.method == ConnectedTextureMethods::FLORA) {
            // Just a bit of algebra to solve for n with the equation y = (n² + n) / 2
            // which becomes n = (sqrt(8 * y + 1) - 1) / 2
            int y = textures[0].base.size.y;
            floraHeight = (ui16)(sqrt(8 * y + 1) - 1) / 2;
        }
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
    for (size_t i = 0; i < Blocks.size(); i++){
        if (Blocks[i].active){
            Blocks[i].SetAvgTexColors();
        }
    }
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
