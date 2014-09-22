#include "stdafx.h"
#include "BlockData.h"

#include "Errors.h"
#include "FileSystem.h"
#include "FrameBuffer.h"
#include "GameManager.h"
#include "TextureAtlasManager.h"
#include "Options.h"
#include "Rendering.h"
#include "TerrainGenerator.h"
#include "Texture2d.h"
#include "ZipFile.h"

KEG_ENUM_INIT_BEGIN(ConnectedTextureMethods, ConnectedTextureMethods, e)
e->addValue("none", ConnectedTextureMethods::CTM_NONE);
e->addValue("connect", ConnectedTextureMethods::CTM_CONNECTED);
e->addValue("random", ConnectedTextureMethods::CTM_RANDOM);
e->addValue("repeat", ConnectedTextureMethods::CTM_REPEAT);
e->addValue("grass", ConnectedTextureMethods::CTM_GRASS);
e->addValue("horizontal", ConnectedTextureMethods::CTM_HORIZONTAL);
e->addValue("vertical", ConnectedTextureMethods::CTM_VERTICAL);
KEG_ENUM_INIT_END

KEG_ENUM_INIT_BEGIN(ConnectedTextureSymmetry, ConnectedTextureSymmetry, e)
e->addValue("none", ConnectedTextureSymmetry::SYMMETRY_NONE);
e->addValue("opposite", ConnectedTextureSymmetry::SYMMETRY_OPPOSITE);
e->addValue("all", ConnectedTextureSymmetry::SYMMETRY_ALL);
KEG_ENUM_INIT_END

KEG_ENUM_INIT_BEGIN(ConnectedTextureReducedMethod, ConnectedTextureReducedMethod, e);
e->addValue("none", ConnectedTextureReducedMethod::NONE);
e->addValue("top", ConnectedTextureReducedMethod::TOP);
e->addValue("bottom", ConnectedTextureReducedMethod::BOTTOM);
KEG_ENUM_INIT_END

KEG_ENUM_INIT_BEGIN(BlendType, BlendType, e)
e->addValue("add", BlendType::BLEND_TYPE_ADD);
e->addValue("multiply", BlendType::BLEND_TYPE_MULTIPLY);
e->addValue("replace", BlendType::BLEND_TYPE_REPLACE);
e->addValue("subtract", BlendType::BLEND_TYPE_SUBTRACT);
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

vector <Block> Blocks;

vector <int> TextureUnitIndices;

int connectedTextureOffsets[256];
int grassTextureOffsets[32];

map <string, BlockVariable> blockVariableMap;

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
    weight = -1;
    value = 0;
    isCrushable = 0;
    floatingAction = 1;
    isSupportive = 1;
    explosivePower = 0.0;
    powerLoss = 0.0;
    explosionRays = 0;
    material = M_NONE;
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

        BlockTexture* blockTexture;

        blockTexture = GameManager::textureAtlasManager->getBlockTexture(topTexName);
        if (blockTexture){
            pyTexInfo = *blockTexture;
            pyTex = blockTexture->base.textureIndex;
            pyOvTex = blockTexture->overlay.textureIndex;
        }
        blockTexture = GameManager::textureAtlasManager->getBlockTexture(leftTexName);
        if (blockTexture){
            nxTexInfo = *blockTexture;
            nxTex = blockTexture->base.textureIndex;
            nxOvTex = blockTexture->overlay.textureIndex;
        }
        blockTexture = GameManager::textureAtlasManager->getBlockTexture(rightTexName);
        if (blockTexture){
            pxTexInfo = *blockTexture;
            pxTex = blockTexture->base.textureIndex;
            pxOvTex = blockTexture->overlay.textureIndex;
        }
        blockTexture = GameManager::textureAtlasManager->getBlockTexture(frontTexName);
        if (blockTexture){
            pzTexInfo = *blockTexture;
            pzTex = blockTexture->base.textureIndex;
            pzOvTex = blockTexture->overlay.textureIndex;
        }
        blockTexture = GameManager::textureAtlasManager->getBlockTexture(backTexName);
        if (blockTexture){
            nzTexInfo = *blockTexture;
            nzTex = blockTexture->base.textureIndex;
            nzOvTex = blockTexture->overlay.textureIndex;
        }
        blockTexture = GameManager::textureAtlasManager->getBlockTexture(bottomTexName);
        if (blockTexture){
            nyTexInfo = *blockTexture;
            nyTex = blockTexture->base.textureIndex;
            nyOvTex = blockTexture->overlay.textureIndex;
        }
        blockTexture = GameManager::textureAtlasManager->getBlockTexture(particleTexName);
        if (blockTexture){
            particleTex = blockTexture->base.textureIndex;
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
    //Vertex verts[48];
    //int vertexOffset = 0;
    //int x = 0.1, y = 0.1, z = 0.1;
    //int waveEffect = block->waveEffect;
    //int tex = block->pyTex;
    //int texUnit = block->topTexUnit;
    //float lightMult = 0.95f, lightOff = -0.19f;

    //glm::ivec3 color;

    //if (flags && block->altColors.size()){
    //    color = block->altColors[flags];
    //}else{
    //    color = glm::ivec3(block->tr, block->tg, block->tb);
    //}

    //for (int index = 0; index < 24; index+=4, vertexOffset += 12){
    //    verts[index].location[0] = x+cubeVertices[vertexOffset]*0.8;
    //    verts[index].location[1] = y+cubeVertices[vertexOffset+1]*0.8;
    //    verts[index].location[2] = z+cubeVertices[vertexOffset+2]*0.8;
    //    verts[index+1].location[0] = x+cubeVertices[vertexOffset+3]*0.8;
    //    verts[index+1].location[1] = y+cubeVertices[vertexOffset+4]*0.8;
    //    verts[index+1].location[2] = z+cubeVertices[vertexOffset+5]*0.8;
    //    verts[index+2].location[0] = x+cubeVertices[vertexOffset+6]*0.8;
    //    verts[index+2].location[1] = y+cubeVertices[vertexOffset+7]*0.8;
    //    verts[index+2].location[2] = z+cubeVertices[vertexOffset+8]*0.8;
    //    verts[index+3].location[0] = x+cubeVertices[vertexOffset+9]*0.8;
    //    verts[index+3].location[1] = y+cubeVertices[vertexOffset+10]*0.8;
    //    verts[index+3].location[2] = z+cubeVertices[vertexOffset+11]*0.8;

    //    verts[index].light = (GLubyte)(255.0f*(lightOff+pow(lightMult, MAXLIGHT - light)));
    //    verts[index].sunLight = (GLubyte)(255.0f*(lightOff+pow(lightMult, MAXLIGHT - sunlight)));
    //    verts[index+1].light = (GLubyte)(255.0f*(lightOff+pow(lightMult, MAXLIGHT - light)));
    //    verts[index+1].sunLight = (GLubyte)(255.0f*(lightOff+pow(lightMult, MAXLIGHT - sunlight)));
    //    verts[index+2].light = (GLubyte)(255.0f*(lightOff+pow(lightMult, MAXLIGHT - light)));
    //    verts[index+2].sunLight = (GLubyte)(255.0f*(lightOff+pow(lightMult, MAXLIGHT - sunlight)));
    //    verts[index+3].light = (GLubyte)(255.0f*(lightOff+pow(lightMult, MAXLIGHT - light)));
    //    verts[index+3].sunLight = (GLubyte)(255.0f*(lightOff+pow(lightMult, MAXLIGHT - sunlight)));


    //    verts[index].color[0] = color.r;
    //    verts[index+1].color[0] = color.r;
    //    verts[index+2].color[0] = color.r;
    //    verts[index+3].color[0] = color.r;
    //    verts[index].color[1] = color.g;
    //    verts[index+1].color[1] = color.g;
    //    verts[index+2].color[1] = color.g;
    //    verts[index+3].color[1] = color.g;
    //    verts[index].color[2] = color.b;
    //    verts[index+1].color[2] = color.b;
    //    verts[index+2].color[2] = color.b;
    //    verts[index+3].color[2] = color.b;
    //    if (waveEffect == 2){
    //        verts[index].color[3] = 255;
    //        verts[index+1].color[3] = 255;
    //        verts[index+2].color[3] = 255;
    //        verts[index+3].color[3] = 255;
    //    }else if (waveEffect == 1){
    //        verts[index].color[3] = 255;
    //        verts[index+1].color[3] = 0;
    //        verts[index+2].color[3] = 0;
    //        verts[index+3].color[3] = 255;
    //    }else{
    //        verts[index].color[3] = 0;
    //        verts[index+1].color[3] = 0;
    //        verts[index+2].color[3] = 0;
    //        verts[index+3].color[3] = 0;
    //    }

    //    verts[index].tex[0] = 0.0f;
    //    verts[index].tex[1] = 1.0f;
    //    verts[index+1].tex[0] = 0.0f;
    //    verts[index+1].tex[1] = 0.0f;
    //    verts[index+2].tex[0] = 1.0f;
    //    verts[index+2].tex[1] = 0.0f;
    //    verts[index+3].tex[0] = 1.0f;
    //    verts[index+3].tex[1] = 1.0f;
    //    verts[index].textureLocation = (GLubyte)blockTextureUVs[tex].textureLocation;
    //    verts[index+1].textureLocation = (GLubyte)blockTextureUVs[tex].textureLocation;
    //    verts[index+2].textureLocation = (GLubyte)blockTextureUVs[tex].textureLocation;
    //    verts[index+3].textureLocation = (GLubyte)blockTextureUVs[tex].textureLocation;

    //    verts[index].textureUnit = (GLfloat)texUnit;
    //    verts[index+1].textureUnit = (GLfloat)texUnit;
    //    verts[index+2].textureUnit = (GLfloat)texUnit;
    //    verts[index+3].textureUnit = (GLfloat)texUnit;
    //}
    //x = y = z = -0.5;
    //vertexOffset = 0;
    //for (int index = 24; index < 48; index+=4, vertexOffset += 12){
    //    verts[index].location[0] = x+cubeVertices[vertexOffset]*1.5;
    //    verts[index].location[1] = y+cubeVertices[vertexOffset+1]*1.5;
    //    verts[index].location[2] = z+cubeVertices[vertexOffset+2]*1.5;
    //    verts[index+1].location[0] = x+cubeVertices[vertexOffset+3]*1.5;
    //    verts[index+1].location[1] = y+cubeVertices[vertexOffset+4]*1.5;
    //    verts[index+1].location[2] = z+cubeVertices[vertexOffset+5]*1.5;
    //    verts[index+2].location[0] = x+cubeVertices[vertexOffset+6]*1.5;
    //    verts[index+2].location[1] = y+cubeVertices[vertexOffset+7]*1.5;
    //    verts[index+2].location[2] = z+cubeVertices[vertexOffset+8]*1.5;
    //    verts[index+3].location[0] = x+cubeVertices[vertexOffset+9]*1.5;
    //    verts[index+3].location[1] = y+cubeVertices[vertexOffset+10]*1.5;
    //    verts[index+3].location[2] = z+cubeVertices[vertexOffset+11]*1.5;

    //    verts[index].light = (GLubyte)(255.0f*(lightOff+pow(lightMult, MAXLIGHT - light)));
    //    verts[index].sunLight = (GLubyte)(255.0f*(lightOff+pow(lightMult, MAXLIGHT - sunlight)));
    //    verts[index+1].light = (GLubyte)(255.0f*(lightOff+pow(lightMult, MAXLIGHT - light)));
    //    verts[index+1].sunLight = (GLubyte)(255.0f*(lightOff+pow(lightMult, MAXLIGHT - sunlight)));
    //    verts[index+2].light = (GLubyte)(255.0f*(lightOff+pow(lightMult, MAXLIGHT - light)));
    //    verts[index+2].sunLight = (GLubyte)(255.0f*(lightOff+pow(lightMult, MAXLIGHT - sunlight)));
    //    verts[index+3].light = (GLubyte)(255.0f*(lightOff+pow(lightMult, MAXLIGHT - light)));
    //    verts[index+3].sunLight = (GLubyte)(255.0f*(lightOff+pow(lightMult, MAXLIGHT - sunlight)));


    //    verts[index].color[0] = color.r;
    //    verts[index+1].color[0] = color.r;
    //    verts[index+2].color[0] = color.r;
    //    verts[index+3].color[0] = color.r;
    //    verts[index].color[1] = color.g;
    //    verts[index+1].color[1] = color.g;
    //    verts[index+2].color[1] = color.g;
    //    verts[index+3].color[1] = color.g;
    //    verts[index].color[2] = color.b;
    //    verts[index+1].color[2] = color.b;
    //    verts[index+2].color[2] = color.b;
    //    verts[index+3].color[2] = color.b;
    //    if (waveEffect == 2){
    //        verts[index].color[3] = 255;
    //        verts[index+1].color[3] = 255;
    //        verts[index+2].color[3] = 255;
    //        verts[index+3].color[3] = 255;
    //    }else if (waveEffect == 1){
    //        verts[index].color[3] = 255;
    //        verts[index+1].color[3] = 0;
    //        verts[index+2].color[3] = 0;
    //        verts[index+3].color[3] = 255;
    //    }else{
    //        verts[index].color[3] = 0;
    //        verts[index+1].color[3] = 0;
    //        verts[index+2].color[3] = 0;
    //        verts[index+3].color[3] = 0;
    //    }

    //    verts[index].tex[0] = 0.0f;
    //    verts[index].tex[1] = 1.0f;
    //    verts[index+1].tex[0] = 0.0f;
    //    verts[index+1].tex[1] = 0.0f;
    //    verts[index+2].tex[0] = 1.0f;
    //    verts[index+2].tex[1] = 0.0f;
    //    verts[index+3].tex[0] = 1.0f;
    //    verts[index+3].tex[1] = 1.0f;
    //    verts[index].textureLocation = (GLubyte)blockTextureUVs[tex].textureLocation;
    //    verts[index+1].textureLocation = (GLubyte)blockTextureUVs[tex].textureLocation;
    //    verts[index+2].textureLocation = (GLubyte)blockTextureUVs[tex].textureLocation;
    //    verts[index+3].textureLocation = (GLubyte)blockTextureUVs[tex].textureLocation;

    //    verts[index].textureUnit = (GLfloat)texUnit;
    //    verts[index+1].textureUnit = (GLfloat)texUnit;
    //    verts[index+2].textureUnit = (GLfloat)texUnit;
    //    verts[index+3].textureUnit = (GLfloat)texUnit;
    //}

    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, blockPacks[0]);
    //glActiveTexture(GL_TEXTURE1);
    //glBindTexture(GL_TEXTURE_2D, blockPacks[1]);

    //GLuint vboID;
    //glGenBuffers(1, &vboID);
    //glBindBuffer(GL_ARRAY_BUFFER, vboID);
    //glBufferData(GL_ARRAY_BUFFER, 48*sizeof(Vertex), NULL, GL_STREAM_DRAW);
    //glBufferSubData(GL_ARRAY_BUFFER, 0, 48*sizeof(Vertex), verts);

    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Chunk::vboIndicesID);

    //GlobalModelMatrix[3][0] = ((float)(floor(position.x) - position.x));
    //GlobalModelMatrix[3][1] = ((float)(floor(position.y) - position.y));
    //GlobalModelMatrix[3][2] = ((float)(floor(position.z) - position.z));

    //totDrawCalls++;
    //totVertices += 48;
    //glm::mat4 MVP = VP * GlobalModelMatrix;
    //
    //glUniformMatrix4fv(blockShader.mvpID, 1, GL_FALSE, &MVP[0][0]);
    //glUniformMatrix4fv(blockShader.mID, 1, GL_FALSE, &GlobalModelMatrix[0][0]);

    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    ////UVs
    //glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), ((char *)NULL + (12)));
    ////colors
    //glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), ((char *)NULL + (20)));
    ////textureLocation, light, sunlight, texture unit
    //glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), ((char *)NULL + (24))); 

    ////vertices
    //glDrawElements(GL_TRIANGLES, 72, GL_UNSIGNED_INT, 0);

    //glBindBuffer(GL_ARRAY_BUFFER, 0);
    //glDeleteBuffers(1, &vboID);
}
/*
GLubyte collide;
GLubyte waterBreak;
GLubyte isLight;
GLubyte blockLight;
GLubyte active;
GLubyte useable;
GLubyte allowLight;
GLubyte isCrushable;
GLubyte isSupportive;
GLubyte padding;
*/
