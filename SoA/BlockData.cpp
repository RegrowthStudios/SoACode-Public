#include "stdafx.h"
#include "BlockData.h"

#include "Errors.h"
#include "FileSystem.h"
#include "FrameBuffer.h"
#include "GameManager.h"
#include "Options.h"
#include "Rendering.h"
#include "TerrainGenerator.h"
#include "TexturePackLoader.h"
#include "Texture2d.h"
#include "ZipFile.h"

KEG_ENUM_INIT_BEGIN(MeshType, MeshType, e)
e->addValue("none", MeshType::NONE);
e->addValue("cube", MeshType::BLOCK);
e->addValue("leaves", MeshType::LEAVES);
e->addValue("triangle", MeshType::FLORA);
e->addValue("cross", MeshType::CROSSFLORA);
e->addValue("liquid", MeshType::LIQUID);
e->addValue("flat", MeshType::FLAT);
KEG_ENUM_INIT_END

KEG_ENUM_INIT_BEGIN(ConnectedTextureMethods, ConnectedTextureMethods, e)
e->addValue("none", ConnectedTextureMethods::NONE);
e->addValue("connect", ConnectedTextureMethods::CONNECTED);
e->addValue("random", ConnectedTextureMethods::RANDOM);
e->addValue("repeat", ConnectedTextureMethods::REPEAT);
e->addValue("grass", ConnectedTextureMethods::GRASS);
e->addValue("horizontal", ConnectedTextureMethods::HORIZONTAL);
e->addValue("vertical", ConnectedTextureMethods::VERTICAL);
e->addValue("flora", ConnectedTextureMethods::FLORA);
KEG_ENUM_INIT_END

KEG_ENUM_INIT_BEGIN(ConnectedTextureSymmetry, ConnectedTextureSymmetry, e)
e->addValue("none", ConnectedTextureSymmetry::NONE);
e->addValue("opposite", ConnectedTextureSymmetry::OPPOSITE);
e->addValue("all", ConnectedTextureSymmetry::ALL);
KEG_ENUM_INIT_END

KEG_ENUM_INIT_BEGIN(ConnectedTextureReducedMethod, ConnectedTextureReducedMethod, e);
e->addValue("none", ConnectedTextureReducedMethod::NONE);
e->addValue("top", ConnectedTextureReducedMethod::TOP);
e->addValue("bottom", ConnectedTextureReducedMethod::BOTTOM);
KEG_ENUM_INIT_END

KEG_ENUM_INIT_BEGIN(BlendType, BlendType, e)
e->addValue("add", BlendType::ADD);
e->addValue("multiply", BlendType::MULTIPLY);
e->addValue("replace", BlendType::REPLACE);
e->addValue("subtract", BlendType::SUBTRACT);
KEG_ENUM_INIT_END

KEG_TYPE_INIT_BEGIN_DEF_VAR(BlockTextureLayer)
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("method", Keg::Value::custom("ConnectedTextureMethods", offsetof(BlockTextureLayer, method), true));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("reducedMethod", Keg::Value::custom("ConnectedTextureReducedMethod", offsetof(BlockTextureLayer, reducedMethod), true));
KEG_TYPE_INIT_ADD_MEMBER(BlockTextureLayer, I32_V2, size);
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("width", Keg::Value::basic(Keg::BasicType::I32, offsetof(BlockTextureLayer, size)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("height", Keg::Value::basic(Keg::BasicType::I32, offsetof(BlockTextureLayer, size) + sizeof(i32)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("symmetry", Keg::Value::custom("ConnectedTextureSymmetry", offsetof(BlockTextureLayer, symmetry), true));
KEG_TYPE_INIT_ADD_MEMBER(BlockTextureLayer, BOOL, innerSeams);
KEG_TYPE_INIT_ADD_MEMBER(BlockTextureLayer, BOOL, transparency);
KEG_TYPE_INIT_ADD_MEMBER(BlockTextureLayer, STRING, useMapColor);
KEG_TYPE_INIT_ADD_MEMBER(BlockTextureLayer, I32, totalWeight);
KEG_TYPE_INIT_ADD_MEMBER(BlockTextureLayer, I32, numTiles);
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("weights", Keg::Value::array(offsetof(BlockTextureLayer, weights), Keg::BasicType::I32));
KEG_TYPE_INIT_ADD_MEMBER(BlockTextureLayer, I32, textureIndex);
KEG_TYPE_INIT_ADD_MEMBER(BlockTextureLayer, STRING, path);
KEG_TYPE_INIT_END

KEG_TYPE_INIT_BEGIN_DEF_VAR(BlockTexture)
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("base", Keg::Value::custom("BlockTextureLayer", offsetof(BlockTexture, base)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("overlay", Keg::Value::custom("BlockTextureLayer", offsetof(BlockTexture, overlay)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("blendMode", Keg::Value::custom("BlendType", offsetof(BlockTexture, blendMode), true));
KEG_TYPE_INIT_END

KEG_TYPE_INIT_BEGIN_DEF_VAR(Block)
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("ID", Keg::Value::basic(Keg::BasicType::UI16, offsetof(Block, ID)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("burnTransformID", Keg::Value::basic(Keg::BasicType::UI16, offsetof(Block, burnTransformID)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("waveEffect", Keg::Value::basic(Keg::BasicType::I16, offsetof(Block, waveEffect)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("lightColor", Keg::Value::basic(Keg::BasicType::UI16, offsetof(Block, lightColor)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("physicsProperty", Keg::Value::basic(Keg::BasicType::I16, offsetof(Block, physicsProperty)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("waterMeshLevel", Keg::Value::basic(Keg::BasicType::I16, offsetof(Block, waterMeshLevel)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("floatingAction", Keg::Value::basic(Keg::BasicType::I16, offsetof(Block, floatingAction)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("occlude", Keg::Value::basic(Keg::BasicType::I16, offsetof(Block, occlude)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("spawnerVal", Keg::Value::basic(Keg::BasicType::UI16, offsetof(Block, spawnerVal)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("sinkVal", Keg::Value::basic(Keg::BasicType::UI16, offsetof(Block, sinkVal)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("explosionRays", Keg::Value::basic(Keg::BasicType::UI16, offsetof(Block, explosionRays)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("meshType", Keg::Value::custom("MeshType", offsetof(Block, meshType), true));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("moveMod", Keg::Value::basic(Keg::BasicType::F32, offsetof(Block, moveMod)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("explosionResistance", Keg::Value::basic(Keg::BasicType::F32, offsetof(Block, explosionResistance)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("explosivePower", Keg::Value::basic(Keg::BasicType::F32, offsetof(Block, explosivePower)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("flammability", Keg::Value::basic(Keg::BasicType::F32, offsetof(Block, flammability)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("powerLoss", Keg::Value::basic(Keg::BasicType::F32, offsetof(Block, powerLoss)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("colorFilter", Keg::Value::basic(Keg::BasicType::F32_V3, offsetof(Block, colorFilter)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("color", Keg::Value::basic(Keg::BasicType::UI8_V3, offsetof(Block, color)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("overlayColor", Keg::Value::basic(Keg::BasicType::UI8_V3, offsetof(Block, overlayColor)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("particleTex", Keg::Value::basic(Keg::BasicType::UI8, offsetof(Block, particleTex)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("powderMove", Keg::Value::basic(Keg::BasicType::UI8, offsetof(Block, powderMove)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("collide", Keg::Value::basic(Keg::BasicType::UI8, offsetof(Block, collide)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("waterBreak", Keg::Value::basic(Keg::BasicType::UI8, offsetof(Block, waterBreak)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("isLight", Keg::Value::basic(Keg::BasicType::UI8, offsetof(Block, isLight)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("blockLight", Keg::Value::basic(Keg::BasicType::UI8, offsetof(Block, blockLight)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("useable", Keg::Value::basic(Keg::BasicType::UI8, offsetof(Block, useable)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("allowLight", Keg::Value::basic(Keg::BasicType::UI8, offsetof(Block, allowLight)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("isCrushable", Keg::Value::basic(Keg::BasicType::UI8, offsetof(Block, isCrushable)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("isSupportive", Keg::Value::basic(Keg::BasicType::UI8, offsetof(Block, isSupportive)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("leftTexName", Keg::Value::basic(Keg::BasicType::STRING, offsetof(Block, leftTexName)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("rightTexName", Keg::Value::basic(Keg::BasicType::STRING, offsetof(Block, rightTexName)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("frontTexName", Keg::Value::basic(Keg::BasicType::STRING, offsetof(Block, frontTexName)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("backTexName", Keg::Value::basic(Keg::BasicType::STRING, offsetof(Block, backTexName)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("topTexName", Keg::Value::basic(Keg::BasicType::STRING, offsetof(Block, topTexName)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("bottomTexName", Keg::Value::basic(Keg::BasicType::STRING, offsetof(Block, bottomTexName)));
KEG_TYPE_INIT_END

vector <Block> Blocks;

vector <int> TextureUnitIndices;

int connectedTextureOffsets[256];
int grassTextureOffsets[32];

map <string, BlockVariable> blockVariableMap;

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
    LCHECK(weights.length());
    LCHECK(totalWeight);
    LCHECK(numTiles);
    LCHECK(innerSeams);
    LCHECK(transparency);
    return false;
}

void initConnectedTextures()
{
    memset(connectedTextureOffsets, 0, sizeof(connectedTextureOffsets));
    memset(grassTextureOffsets, 0, sizeof(grassTextureOffsets));

    connectedTextureOffsets[0xFF] = 0;
    connectedTextureOffsets[0xEF] = 1;
    connectedTextureOffsets[0xEE] = 2;
    connectedTextureOffsets[0xFE] = 3;
    connectedTextureOffsets[0xEB] = 4;
    connectedTextureOffsets[0xFA] = 5;
    connectedTextureOffsets[0xAB] = 6;
    connectedTextureOffsets[0xEA] = 7;
    connectedTextureOffsets[0x8A] = 8;
    connectedTextureOffsets[0xA2] = 9;
    connectedTextureOffsets[0x28] = 10;
    connectedTextureOffsets[0xA] = 11;
    connectedTextureOffsets[0xFB] = 12;
    connectedTextureOffsets[0xE3] = 13;
    connectedTextureOffsets[0xE0] = 14;
    connectedTextureOffsets[0xF8] = 15;
    connectedTextureOffsets[0xAF] = 16;
    connectedTextureOffsets[0xBE] = 17;
    connectedTextureOffsets[0xAE] = 18;
    connectedTextureOffsets[0xBA] = 19;
    connectedTextureOffsets[0x2A] = 20;
    connectedTextureOffsets[0xA8] = 21;
    connectedTextureOffsets[0xA0] = 22;
    connectedTextureOffsets[0x82] = 23;
    connectedTextureOffsets[0xBB] = 24;
    connectedTextureOffsets[0x83] = 25;
    connectedTextureOffsets[0] = 26;
    connectedTextureOffsets[0x38] = 27;
    connectedTextureOffsets[0xA3] = 28;
    connectedTextureOffsets[0xE8] = 29;
    connectedTextureOffsets[0x8B] = 30;
    connectedTextureOffsets[0xE2] = 31;
    connectedTextureOffsets[0x8] = 32;
    connectedTextureOffsets[0x2] = 33;
    connectedTextureOffsets[0x88] = 34;
    connectedTextureOffsets[0x22] = 35;
    connectedTextureOffsets[0xBF] = 36;
    connectedTextureOffsets[0x8F] = 37;
    connectedTextureOffsets[0xE] = 38;
    connectedTextureOffsets[0x3E] = 39;
    connectedTextureOffsets[0x8E] = 40;
    connectedTextureOffsets[0x3A] = 41;
    connectedTextureOffsets[0x2E] = 42;
    connectedTextureOffsets[0xB8] = 43;
    connectedTextureOffsets[0x20] = 44;
    connectedTextureOffsets[0x80] = 45;
    connectedTextureOffsets[0xAA] = 46;

    grassTextureOffsets[0x1 | 0x8] = 1;
    grassTextureOffsets[0x8] = 2;
    grassTextureOffsets[0x0] = 3;
    grassTextureOffsets[0x1 | 0x4 | 0x8] = 4;
    grassTextureOffsets[0x1 | 0x2 | 0x8] = 5;
    grassTextureOffsets[0x1 | 0x2 | 0x4 | 0x8] = 6;
    grassTextureOffsets[0x4 | 0x8] = 7;
    grassTextureOffsets[0x1 | 0x2] = 8;
}

Block::Block() : emitterName(""), emitterOnBreakName(""), emitter(NULL), emitterOnBreak(NULL), emitterRandom(NULL), emitterRandomName(""){
    memset(this, 0, sizeof(Block)-sizeof(itemDrops)-sizeof(altColors)); //zero the memory, just in case NOT SURE HOW VECTORS REACT TO THIS
    allowLight = 0;
    ID = 0;
    name = leftTexName = rightTexName = backTexName = frontTexName = topTexName = bottomTexName = particleTexName = "";
    particleTex = 0;
    color[0] = color[1] = color[2] = overlayColor[0] = overlayColor[1] = overlayColor[2] = 255;
    collide = 1;
    occlude = 1;
    meshType = MeshType::BLOCK;
    waveEffect = 0;
    explosionResistance = 1.0;
    active = 0;
    useable = 1;
    blockLight = 1;
    waterMeshLevel = 0;
    isLight = 0;
    lightColor = 0;
    waterBreak = 0;
    isCrushable = 0;
    floatingAction = 1;
    isSupportive = 1;
    explosivePower = 0.0;
    powerLoss = 0.0;
    explosionRays = 0;
    physicsProperty = P_NONE;
    powderMove = 0;
    moveMod = 1.0f;
    spawnerVal = 0;
    sinkVal = 0;
    colorFilter = f32v3(1.0f);
}

void Block::GetBlockColor(GLubyte baseColor[3], GLubyte overlayColor[3], GLuint flags, int temperature, int rainfall, const BlockTexture& blockTexture)
{
    //base color
    if (!blockTexture.base.useMapColor.empty()){
        getTerrainHeightColor(baseColor, temperature, rainfall);
     
        //Average the map color with the base color
        baseColor[0] = (GLubyte)(((float)baseColor[0] * (float)color[0]) / 255.0f);
        baseColor[1] = (GLubyte)(((float)baseColor[1] * (float)color[1]) / 255.0f);
        baseColor[2] = (GLubyte)(((float)baseColor[2] * (float)color[2]) / 255.0f);
    } else if (altColors.size() >= flags && flags){ //alt colors, for leaves and such
        baseColor[0] = altColors[flags - 1].r;
        baseColor[1] = altColors[flags - 1].g;
        baseColor[2] = altColors[flags - 1].b;
    } else{
        baseColor[0] = color[0];
        baseColor[1] = color[1];
        baseColor[2] = color[2];
    }

    //overlay color
    if (!blockTexture.overlay.useMapColor.empty()){
        getTerrainHeightColor(overlayColor, temperature, rainfall);

        //Average the map color with the base color
        overlayColor[0] = (GLubyte)(((float)overlayColor[0] * (float)Block::overlayColor[0]) / 255.0f);
        overlayColor[1] = (GLubyte)(((float)overlayColor[1] * (float)Block::overlayColor[1]) / 255.0f);
        overlayColor[2] = (GLubyte)(((float)overlayColor[2] * (float)Block::overlayColor[2]) / 255.0f);
    } else if (altColors.size() >= flags && flags){ //alt colors, for leaves and such
        overlayColor[0] = altColors[flags - 1].r;
        overlayColor[1] = altColors[flags - 1].g;
        overlayColor[2] = altColors[flags - 1].b;
    } else{
        overlayColor[0] = Block::overlayColor[0];
        overlayColor[1] = Block::overlayColor[1];
        overlayColor[2] = Block::overlayColor[2];
    }
}

void Block::GetBlockColor(GLubyte baseColor[3], GLuint flags, int temperature, int rainfall, const BlockTexture& blockTexture)
{
    //base color
    if (!blockTexture.base.useMapColor.empty()){
        getTerrainHeightColor(baseColor, temperature, rainfall);

        //Average the map color with the base color
        baseColor[0] = (GLubyte)(((float)baseColor[0] * (float)color[0]) / 255.0f);
        baseColor[1] = (GLubyte)(((float)baseColor[1] * (float)color[1]) / 255.0f);
        baseColor[2] = (GLubyte)(((float)baseColor[2] * (float)color[2]) / 255.0f);
    } else if (altColors.size() >= flags && flags){ //alt colors, for leaves and such
        baseColor[0] = altColors[flags - 1].r;
        baseColor[1] = altColors[flags - 1].g;
        baseColor[2] = altColors[flags - 1].b;
    } else{
        baseColor[0] = color[0];
        baseColor[1] = color[1];
        baseColor[2] = color[2];
    }
}

void Block::InitializeTexture()
{
    if (active){
        pxTex = pyTex = pzTex = nxTex = nyTex = nzTex = 0;
        pxNMap = pyNMap = pzNMap = nxNMap = nyNMap = nzNMap = -1;

        GameManager::texturePackLoader->getBlockTexture(topTexName, pyTexInfo);
        pyTex = pyTexInfo.base.textureIndex;
        pyOvTex = pyTexInfo.overlay.textureIndex;
        
        GameManager::texturePackLoader->getBlockTexture(leftTexName, nxTexInfo);
        nxTex = nxTexInfo.base.textureIndex;
        nxOvTex = nxTexInfo.overlay.textureIndex;
        
        GameManager::texturePackLoader->getBlockTexture(rightTexName, pxTexInfo);
        pxTex = pxTexInfo.base.textureIndex;
        pxOvTex = pxTexInfo.overlay.textureIndex;
        
        GameManager::texturePackLoader->getBlockTexture(frontTexName, pzTexInfo);
        pzTex = pzTexInfo.base.textureIndex;
        pzOvTex = pzTexInfo.overlay.textureIndex;
        
        GameManager::texturePackLoader->getBlockTexture(backTexName, nzTexInfo);
        nzTex = nzTexInfo.base.textureIndex;
        nzOvTex = nzTexInfo.overlay.textureIndex;
        
        GameManager::texturePackLoader->getBlockTexture(bottomTexName, nyTexInfo);
        nyTex = nyTexInfo.base.textureIndex;
        nyOvTex = nyTexInfo.overlay.textureIndex;
        
        BlockTexture particleTexture;
        GameManager::texturePackLoader->getBlockTexture(particleTexName, particleTexture);
        particleTex = particleTexture.base.textureIndex;
        
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
