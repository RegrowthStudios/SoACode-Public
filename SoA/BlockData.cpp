#include "stdafx.h"
#include "BlockData.h"

#include "Errors.h"
#include "FileSystem.h"
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

KEG_ENUM_INIT_BEGIN(PhysicsProperties, PhysicsProperties, e);
e->addValue("none", PhysicsProperties::P_NONE);
e->addValue("solid", PhysicsProperties::P_SOLID);
e->addValue("liquid", PhysicsProperties::P_LIQUID);
e->addValue("powder", PhysicsProperties::P_POWDER);
e->addValue("snow", PhysicsProperties::P_SNOW);
KEG_ENUM_INIT_END

KEG_ENUM_INIT_BEGIN(BlockOcclusion, BlockOcclusion, e);
e->addValue("none", BlockOcclusion::NONE);
e->addValue("self", BlockOcclusion::SELF);
e->addValue("selfOnly", BlockOcclusion::SELF_ONLY);
e->addValue("all", BlockOcclusion::ALL);
KEG_ENUM_INIT_END

KEG_ENUM_INIT_BEGIN(ConnectedTextureReducedMethod, ConnectedTextureReducedMethod, e);
e->addValue("none", ConnectedTextureReducedMethod::NONE);
e->addValue("top", ConnectedTextureReducedMethod::TOP);
e->addValue("bottom", ConnectedTextureReducedMethod::BOTTOM);
KEG_ENUM_INIT_END

KEG_ENUM_INIT_BEGIN(BlendType, BlendType, e)
e->addValue("add", BlendType::ADD);
e->addValue("multiply", BlendType::MULTIPLY);
e->addValue("replace", BlendType::ALPHA);
e->addValue("subtract", BlendType::SUBTRACT);
KEG_ENUM_INIT_END

KEG_TYPE_INIT_BEGIN_DEF_VAR(BlockTextureLayer)
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("method", Keg::Value::custom("ConnectedTextureMethods", offsetof(BlockTextureLayer, method), true));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("reducedMethod", Keg::Value::custom("ConnectedTextureReducedMethod", offsetof(BlockTextureLayer, reducedMethod), true));
KEG_TYPE_INIT_ADD_MEMBER(BlockTextureLayer, I32_V2, size);
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("symmetry", Keg::Value::custom("ConnectedTextureSymmetry", offsetof(BlockTextureLayer, symmetry), true));
KEG_TYPE_INIT_ADD_MEMBER(BlockTextureLayer, BOOL, innerSeams);
KEG_TYPE_INIT_ADD_MEMBER(BlockTextureLayer, BOOL, transparency);
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("height", Keg::Value::basic(Keg::BasicType::UI32, offsetof(BlockTextureLayer, floraHeight)));
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
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("lightColor", Keg::Value::basic(Keg::BasicType::UI8_V3, offsetof(Block, lightColor)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("physicsProperty", Keg::Value::custom("PhysicsProperties", offsetof(Block, physicsProperty), true));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("waterMeshLevel", Keg::Value::basic(Keg::BasicType::I16, offsetof(Block, waterMeshLevel)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("floatingAction", Keg::Value::basic(Keg::BasicType::I16, offsetof(Block, floatingAction)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("occlusion", Keg::Value::custom("BlockOcclusion", offsetof(Block, occlude), true));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("spawnerVal", Keg::Value::basic(Keg::BasicType::UI16, offsetof(Block, spawnerVal)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("sinkVal", Keg::Value::basic(Keg::BasicType::UI16, offsetof(Block, sinkVal)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("explosionRays", Keg::Value::basic(Keg::BasicType::UI16, offsetof(Block, explosionRays)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("meshType", Keg::Value::custom("MeshType", offsetof(Block, meshType), true));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("moveMod", Keg::Value::basic(Keg::BasicType::F32, offsetof(Block, moveMod)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("explosionResistance", Keg::Value::basic(Keg::BasicType::F32, offsetof(Block, explosionResistance)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("explosionPower", Keg::Value::basic(Keg::BasicType::F32, offsetof(Block, explosivePower)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("flammability", Keg::Value::basic(Keg::BasicType::F32, offsetof(Block, flammability)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("explosionPowerLoss", Keg::Value::basic(Keg::BasicType::F32, offsetof(Block, explosionPowerLoss)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("lightColorFilter", Keg::Value::basic(Keg::BasicType::F32_V3, offsetof(Block, colorFilter)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("color", Keg::Value::basic(Keg::BasicType::UI8_V3, offsetof(Block, color)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("emitter", Keg::Value::basic(Keg::BasicType::STRING, offsetof(Block, emitterName)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("movesPowder", Keg::Value::basic(Keg::BasicType::BOOL, offsetof(Block, powderMove)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("collide", Keg::Value::basic(Keg::BasicType::BOOL, offsetof(Block, collide)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("waterBreak", Keg::Value::basic(Keg::BasicType::BOOL, offsetof(Block, waterBreak)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("scatterSunRays", Keg::Value::basic(Keg::BasicType::BOOL, offsetof(Block, blockLight)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("useable", Keg::Value::basic(Keg::BasicType::BOOL, offsetof(Block, useable)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("allowsLight", Keg::Value::basic(Keg::BasicType::BOOL, offsetof(Block, allowLight)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("crushable", Keg::Value::basic(Keg::BasicType::BOOL, offsetof(Block, isCrushable)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("supportive", Keg::Value::basic(Keg::BasicType::BOOL, offsetof(Block, isSupportive)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("textureLeft", Keg::Value::basic(Keg::BasicType::STRING, offsetof(Block, leftTexName)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("textureRight", Keg::Value::basic(Keg::BasicType::STRING, offsetof(Block, rightTexName)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("textureFront", Keg::Value::basic(Keg::BasicType::STRING, offsetof(Block, frontTexName)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("textureBack", Keg::Value::basic(Keg::BasicType::STRING, offsetof(Block, backTexName)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("textureTop", Keg::Value::basic(Keg::BasicType::STRING, offsetof(Block, topTexName)));
KEG_TYPE_INIT_DEF_VAR_NAME->addValue("textureBottom", Keg::Value::basic(Keg::BasicType::STRING, offsetof(Block, bottomTexName)));
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
    LCHECK(weights.getLength());
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

Block::Block() : emitterName(""), 
emitterOnBreakName(""), 
emitter(NULL),
emitterOnBreak(NULL), 
emitterRandom(NULL),
emitterRandomName(""),
color(255, 255, 255),
overlayColor(255, 255, 255),
lightColor(0, 0, 0) {
    allowLight = 0;
    ID = 0;
    name = leftTexName = rightTexName = backTexName = frontTexName = topTexName = bottomTexName = particleTexName = "";
    particleTex = 0;
    collide = 1;
    occlude = BlockOcclusion::ALL;
    meshType = MeshType::BLOCK;
    waveEffect = 0;
    explosionResistance = 1.0;
    active = 0;
    useable = 1;
    blockLight = 1;
    waterMeshLevel = 0;
    waterBreak = 0;
    isCrushable = 0;
    floatingAction = 1;
    flammability = 0.0f;
    isSupportive = 1;
    explosivePower = 0.0;
    explosionPowerLoss = 0.0;
    explosionRays = 0;
    physicsProperty = P_NONE;
    powderMove = 0;
    moveMod = 1.0f;
    spawnerVal = 0;
    sinkVal = 0;
    colorFilter = f32v3(1.0f);
}

void Block::GetBlockColor(ColorRGB8& baseColor, ColorRGB8& overlayColor, GLuint flags, int temperature, int rainfall, const BlockTexture& blockTexture)
{
    //base color
    if (!blockTexture.base.useMapColor.empty()){
        GameManager::texturePackLoader->getColorMapColor(blockTexture.base.colorMapIndex, baseColor, temperature, rainfall);
     
        //Average the map color with the base color
        baseColor.r = (GLubyte)(((float)baseColor.r * (float)color.r) / 255.0f);
        baseColor.g = (GLubyte)(((float)baseColor.g * (float)color.g) / 255.0f);
        baseColor.b = (GLubyte)(((float)baseColor.b * (float)color.b) / 255.0f);
    } else if (altColors.size() >= flags && flags){ //alt colors, for leaves and such
        baseColor = altColors[flags - 1];
    } else{
        baseColor = color;
    }

    //overlay color
    if (!blockTexture.overlay.useMapColor.empty()){
        GameManager::texturePackLoader->getColorMapColor(blockTexture.overlay.colorMapIndex, overlayColor, temperature, rainfall);

        //Average the map color with the base color
        overlayColor.r = (GLubyte)(((float)overlayColor.r * (float)Block::overlayColor.r) / 255.0f);
        overlayColor.g = (GLubyte)(((float)overlayColor.g * (float)Block::overlayColor.g) / 255.0f);
        overlayColor.b = (GLubyte)(((float)overlayColor.b * (float)Block::overlayColor.b) / 255.0f);
    } else if (altColors.size() >= flags && flags){ //alt colors, for leaves and such
        overlayColor= altColors[flags - 1];
    } else{
        overlayColor = Block::overlayColor;
    }
}

void Block::GetBlockColor(ColorRGB8& baseColor, GLuint flags, int temperature, int rainfall, const BlockTexture& blockTexture)
{
    //base color
    if (!blockTexture.base.useMapColor.empty()){

        GameManager::texturePackLoader->getColorMapColor(blockTexture.base.colorMapIndex, baseColor, temperature, rainfall);

        //Average the map color with the base color
        baseColor.r = (GLubyte)(((float)baseColor.r * (float)color.r) / 255.0f);
        baseColor.g = (GLubyte)(((float)baseColor.g * (float)color.g) / 255.0f);
        baseColor.b = (GLubyte)(((float)baseColor.b * (float)color.b) / 255.0f);
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

        GameManager::texturePackLoader->getBlockTexture(topTexName, pyTexInfo);
        base.py = pyTexInfo.base.textureIndex;
        overlay.py = pyTexInfo.overlay.textureIndex;

        GameManager::texturePackLoader->getBlockTexture(leftTexName, nxTexInfo);
        base.nx = nxTexInfo.base.textureIndex;
        overlay.nx = nxTexInfo.overlay.textureIndex;

        GameManager::texturePackLoader->getBlockTexture(rightTexName, pxTexInfo);
        base.px = pxTexInfo.base.textureIndex;
        overlay.px = pxTexInfo.overlay.textureIndex;

        GameManager::texturePackLoader->getBlockTexture(frontTexName, pzTexInfo);
        base.pz = pzTexInfo.base.textureIndex;
        overlay.pz = pzTexInfo.overlay.textureIndex;

        GameManager::texturePackLoader->getBlockTexture(backTexName, nzTexInfo);
        base.nz = nzTexInfo.base.textureIndex;
        overlay.nz = nzTexInfo.overlay.textureIndex;

        GameManager::texturePackLoader->getBlockTexture(bottomTexName, nyTexInfo);
        base.ny = nyTexInfo.base.textureIndex;
        overlay.ny = nyTexInfo.overlay.textureIndex;

        BlockTexture particleTexture;
        GameManager::texturePackLoader->getBlockTexture(particleTexName, particleTexture);
        particleTex = particleTexture.base.textureIndex;

        // Calculate flora height
        // TODO(Ben): Not really a good place for this
        if (pxTexInfo.base.method == ConnectedTextureMethods::FLORA) {
            // Just a bit of algebra to solve for n with the equation y = (n² + n) / 2
            // which becomes n = (sqrt(8 * y + 1) - 1) / 2
            int y = pxTexInfo.base.size.y;
            floraHeight = (sqrt(8 * y + 1) - 1) / 2;
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
