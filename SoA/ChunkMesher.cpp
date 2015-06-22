#include "stdafx.h"
#include "ChunkMesher.h"

#include <random>


#include "Biome.h"
#include "BlockData.h"

#include <Vorb/ThreadPool.h>
#include <Vorb/utils.h>

#include "BlockPack.h"
#include "Chunk.h"
#include "Errors.h"
#include "GameManager.h"
#include "SoaOptions.h"
#include "RenderTask.h"
#include "VoxelMesher.h"
#include "VoxelUtils.h"
#include "VoxelBits.h"

#define GETBLOCK(a) (((*m_blocks)[((a) & 0x0FFF)]))

const float LIGHT_MULT = 0.95f, LIGHT_OFFSET = -0.2f;

const int MAXLIGHT = 31;

void ChunkMesher::init(const BlockPack* blocks) {
    m_blocks = blocks;
}

// TODO(Ben): Better name and functionality please.
void ChunkMesher::bindVBOIndicesID()
{
    std::vector<ui32> indices;
    indices.resize(589824);

    int j = 0;
    for (size_t i = 0; i < indices.size()-12; i += 6){
        indices[i] = j;
        indices[i+1] = j+1;
        indices[i+2] = j+2;
        indices[i+3] = j+2;
        indices[i+4] = j+3;
        indices[i+5] = j;
        j += 4;
    }

    if (Chunk::vboIndicesID != 0){
        glDeleteBuffers(1, &(Chunk::vboIndicesID));
    }
    glGenBuffers(1, &(Chunk::vboIndicesID));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (Chunk::vboIndicesID));
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 500000 * sizeof(GLuint), NULL, GL_STATIC_DRAW);
        
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, 500000 * sizeof(GLuint), &(indices[0]));
}

#define CompareVertices(v1, v2) (!memcmp(&v1.color, &v2.color, 3) && v1.sunlight == v2.sunlight && !memcmp(&v1.lampColor, &v2.lampColor, 3)  \
    && !memcmp(&v1.overlayColor, &v2.overlayColor, 3) \
    && v1.textureAtlas == v2.textureAtlas && v1.textureIndex == v2.textureIndex && v1.overlayTextureAtlas == v2.overlayTextureAtlas && v1.overlayTextureIndex == v2.overlayTextureIndex)

#define CompareVerticesLight(v1, v2) (v1.sunlight == v2.sunlight && !memcmp(&v1.lampColor, &v2.lampColor, 3) && !memcmp(&v1.color, &v2.color, 3))

bool ChunkMesher::shouldRenderFace(int offset) {
    const Block& neighbor = m_blocks->operator[](m_blockData[m_blockIndex + offset]);
    if (neighbor.occlude == BlockOcclusion::ALL) return false;
    if ((neighbor.occlude == BlockOcclusion::SELF) && (m_blockID == neighbor.ID)) return false;
    return true;
}
void ChunkMesher::addBlock()
{
    ColorRGB8 color, overlayColor;
    int textureIndex, overlayTextureIndex;

    GLfloat ambientOcclusion[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    //get bit flags (needs to be changed) -Ben
    //GLuint flags = GETFLAGS(mi.blockIDData[mi.wc]);

    // Check the faces
    // Left
    if (shouldRenderFace(-1)) {
        addQuad((int)vvox::Cardinal::X_NEG, 0, 0);
    }
    // Right
    if (shouldRenderFace(1)) {
        addQuad((int)vvox::Cardinal::X_POS, 0, 0);
    }
    // Bottom
    if (shouldRenderFace(-PADDED_CHUNK_LAYER)) {
        addQuad((int)vvox::Cardinal::Y_NEG, 0, 0);
    }
    // Top
    if (shouldRenderFace(PADDED_CHUNK_LAYER)) {
        addQuad((int)vvox::Cardinal::Y_POS, 0, 0);
    }
    // Back
    if (shouldRenderFace(-PADDED_CHUNK_WIDTH)) {
        addQuad((int)vvox::Cardinal::Z_NEG, 0, 0);
    }
    // Front
    if (shouldRenderFace(PADDED_CHUNK_WIDTH)) {
        addQuad((int)vvox::Cardinal::Z_POS, 0, 0);
    }
}

void ChunkMesher::addQuad(int face, int leftOffset, int downOffset) {
    // Get color
    // TODO(Ben): Flags?
    color3 blockColor[2];
    m_block->getBlockColor(blockColor[0], blockColor[1],
                           0,
                           m_heightData->temperature,
                           m_heightData->rainfall,
                           m_block->textures[face]);

    // TODO(Ben): Merging
    m_quads[face].emplace_back();
    VoxelQuad& quad = m_quads[face].back();
    quad.v0.faceIndex = face; // Useful for later
    for (int i = 0; i < 4; i++) {
        quad.verts[i].position = VoxelMesher::VOXEL_POSITIONS[face][i] + m_voxelPosOffset;
        quad.verts[i].color = blockColor[0];
        quad.verts[i].overlayColor = blockColor[1];
    }
    // TODO(Ben): Think about this more
    if (quad.v0.position.x < m_lowestX) m_lowestX = quad.v0.position.x;
    if (quad.v0.position.x > m_highestX) m_highestX = quad.v0.position.x;
    if (quad.v0.position.y < m_lowestY) m_lowestY = quad.v0.position.y;
    if (quad.v0.position.y > m_highestY) m_highestY = quad.v0.position.y;
    if (quad.v0.position.z < m_lowestZ) m_lowestZ = quad.v0.position.z;
    if (quad.v0.position.z > m_highestZ) m_highestZ = quad.v0.position.z;

}

//adds a flora mesh
void ChunkMesher::addFlora() {

    //const Block &block = (*m_blocks)[mi.btype];
    //mi.currentBlock = &block;

    //ColorRGB8 color, overlayColor;
    //const int wc = mi.wc;
    //const int btype = mi.btype;

    ////Biome *biome = mi.chunkGridData->heightData[mi.nz*CHUNK_WIDTH + mi.nx].biome;
    //int temperature = mi.chunkGridData->heightData[mi.nz*CHUNK_WIDTH + mi.nx].temperature;
    //int rainfall = mi.chunkGridData->heightData[mi.nz*CHUNK_WIDTH + mi.nx].rainfall;

    //GLuint flags = GETFLAGS(m_blockData[mi.wc]);

    //ui8 sunLight = m_sunData[wc];
    //ColorRGB8 lampLight((m_lampData[wc] & LAMP_RED_MASK) >> LAMP_RED_SHIFT,
    //                    (m_lampData[wc] & LAMP_GREEN_MASK) >> LAMP_GREEN_SHIFT,
    //                    m_lampData[wc] & LAMP_BLUE_MASK);

    //sunLight = (ui8)(255.0f*(LIGHT_OFFSET + pow(LIGHT_MULT, MAXLIGHT - sunLight)));
    //lampLight.r = (ui8)(255.0f*(LIGHT_OFFSET + pow(LIGHT_MULT, MAXLIGHT - lampLight.r)));
    //lampLight.g = (ui8)(255.0f*(LIGHT_OFFSET + pow(LIGHT_MULT, MAXLIGHT - lampLight.g)));
    //lampLight.b = (ui8)(255.0f*(LIGHT_OFFSET + pow(LIGHT_MULT, MAXLIGHT - lampLight.b)));

    //(*m_blocks)[btype].GetBlockColor(color, overlayColor, flags, temperature, rainfall, block.textures[0]);

    ////We will offset the texture index based on the texture method
    //i32 textureIndex = block.textures[0]->base.getBlockTextureIndex(mi.pxBaseMethodParams, color);
    //i32 overlayTextureIndex = block.textures[0]->overlay.getBlockTextureIndex(mi.pxOverlayMethodParams, overlayColor);

    //int r;

    //switch (mi.meshType){
    //    case MeshType::LEAVES:

    //        break;
    //    case MeshType::CROSSFLORA:
    //        //Generate a random number between 0 and 3 inclusive
    //        r = std::bind(std::uniform_int_distribution<int>(0, 1), std::mt19937(/*getPositionSeed(mi.nx, mi.nz)*/))();

    //        _cutoutVerts.resize(_cutoutVerts.size() + 8);
    //        VoxelMesher::makeFloraFace(&(_cutoutVerts[0]), VoxelMesher::crossFloraVertices[r], VoxelMesher::floraNormals, 0, block.waveEffect, i32v3(mi.nx, mi.ny, mi.nz), mi.cutoutIndex, textureIndex, overlayTextureIndex, color, overlayColor, sunLight, lampLight, block.textures[0]);
    //        mi.cutoutIndex += 4;
    //        VoxelMesher::makeFloraFace(&(_cutoutVerts[0]), VoxelMesher::crossFloraVertices[r], VoxelMesher::floraNormals, 12, block.waveEffect, i32v3(mi.nx, mi.ny, mi.nz), mi.cutoutIndex, textureIndex, overlayTextureIndex, color, overlayColor, sunLight, lampLight, block.textures[0]);
    //        mi.cutoutIndex += 4;
    //        break;
    //    case MeshType::FLORA:
    //        //Generate a random number between 0 and 3 inclusive
    //        r = std::bind(std::uniform_int_distribution<int>(0, 3), std::mt19937(/*getPositionSeed(mi.nx, mi.nz)*/))();

    //        _cutoutVerts.resize(_cutoutVerts.size() + 12);
    //        VoxelMesher::makeFloraFace(&(_cutoutVerts[0]), VoxelMesher::floraVertices[r], VoxelMesher::floraNormals, 0, block.waveEffect, i32v3(mi.nx, mi.ny, mi.nz), mi.cutoutIndex, textureIndex, overlayTextureIndex, color, overlayColor, sunLight, lampLight, block.textures[0]);
    //        mi.cutoutIndex += 4;
    //        VoxelMesher::makeFloraFace(&(_cutoutVerts[0]), VoxelMesher::floraVertices[r], VoxelMesher::floraNormals, 12, block.waveEffect, i32v3(mi.nx, mi.ny, mi.nz), mi.cutoutIndex, textureIndex, overlayTextureIndex, color, overlayColor, sunLight, lampLight, block.textures[0]);
    //        mi.cutoutIndex += 4;
    //        VoxelMesher::makeFloraFace(&(_cutoutVerts[0]), VoxelMesher::floraVertices[r], VoxelMesher::floraNormals, 24, block.waveEffect, i32v3(mi.nx, mi.ny, mi.nz), mi.cutoutIndex, textureIndex, overlayTextureIndex, color, overlayColor, sunLight, lampLight, block.textures[0]);
    //        mi.cutoutIndex += 4;
    //        break;
    //}
}

//Gets the liquid level from a block index
#define LEVEL(i) ((_blockIDData[i] == 0) ? 0 : (((nextBlock = &GETBLOCK(_blockIDData[i]))->caIndex == block.caIndex) ? nextBlock->waterMeshLevel : 0))

#define CALCULATE_LIQUID_VERTEX_HEIGHT(height, heightA, heightB, cornerIndex) \
    div = 0; \
    tot = 0; \
    if (heightA) { \
        tot += heightA; \
        div++; \
    } \
    \
    if (heightB) { \
        tot += heightB; \
        div++; \
    } \
    \
    if (div) { \
        int lvl = LEVEL(cornerIndex); \
        if (lvl) { \
            tot += lvl; \
            div++; \
        } \
        height = (height + (tot / (float)maxLevel)) / (div + 1); \
    } 
    
   
//END CALCULATE_LIQUID_VERTEX_HEIGHT

// TODO(Ben): Instead of 8 verts per box, share vertices and index into it!
void ChunkMesher::addLiquid(MesherInfo& mi) {
  //  const Block &block = (*m_blocks)[mi.btype];
  //  Block* nextBlock;
  //  i32 nextBlockID;
  //  const i32 wc = mi.wc;
  //  i32 x = mi.x;
  //  i32 y = mi.y;
  //  i32 z = mi.z;

  //  RenderTask* task = mi.task;

  //  const i32 maxLevel = 100;

  //  float liquidLevel = block.waterMeshLevel / (float)maxLevel;
  //  float fallingReduction = 0.0f;

  //  bool faces[6] = { false, false, false, false, false, false };

  //  float backLeftHeight = liquidLevel;
  //  float backRightHeight = liquidLevel;
  //  float frontRightHeight = liquidLevel;
  //  float frontLeftHeight = liquidLevel;

  //  const i32 MIN_ALPHA = 75;
  //  const i32 MAX_ALPHA = 175;
  //  const i32 ALPHA_RANGE = MAX_ALPHA - MIN_ALPHA;

  //  ui8 backRightAlpha, frontRightAlpha, frontLeftAlpha, backLeftAlpha;

  //  i32 textureUnit = 0;

  //  i32 div;
  //  i32 tot;

  //  i32 left, right, back, front, bottom;

  //  ui8 uOff = x * 7;
  //  ui8 vOff = 224 - z * 7;

  //  ui8 temperature = chunkGridData->heightData[x + z*CHUNK_WIDTH].temperature;
  //  ui8 depth = chunkGridData->heightData[x + z*CHUNK_WIDTH].depth;

  //  ColorRGB8 color;// = GameManager::texturePackLoader->getColorMap(TerrainGenerator::DefaultColorMaps::WATER)[depth * 256 + temperature];

  //  ui8 sunlight = _sunlightData[wc];
  //  ColorRGB8 lampLight((_lampLightData[wc] & LAMP_RED_MASK) >> LAMP_RED_SHIFT,
  //                      (_lampLightData[wc] & LAMP_GREEN_MASK) >> LAMP_GREEN_SHIFT,
  //                      _lampLightData[wc] & LAMP_BLUE_MASK);

  //  sunlight = (ui8)(255.0f*(LIGHT_OFFSET + pow(LIGHT_MULT, MAXLIGHT - sunlight)));
  //  lampLight.r = (ui8)(255.0f*(LIGHT_OFFSET + pow(LIGHT_MULT, MAXLIGHT - lampLight.r)));
  //  lampLight.g = (ui8)(255.0f*(LIGHT_OFFSET + pow(LIGHT_MULT, MAXLIGHT - lampLight.g)));
  //  lampLight.b = (ui8)(255.0f*(LIGHT_OFFSET + pow(LIGHT_MULT, MAXLIGHT - lampLight.b)));

  //  nextBlockID = _blockIDData[wc + PADDED_OFFSETS::BOTTOM];
  //  nextBlock = &GETBLOCK(nextBlockID);
  //  //Check if the block is falling
  //  if (nextBlockID == 0 || nextBlock->waterBreak || (nextBlock->caIndex == block.caIndex && nextBlock->waterMeshLevel != maxLevel)) {
  //      memset(faces, 1, 6); //all faces are active
  ////      backLeftHeight = backRightHeight = frontLeftHeight = frontRightHeight 
  //      fallingReduction = 1.0f;
  //  } else {

  //      //Get occlusion
  //      nextBlock = &GETBLOCK(_blockIDData[wc + PADDED_OFFSETS::LEFT]);
  //      faces[XNEG] = ((nextBlock->caIndex != block.caIndex) && (nextBlock->occlude == BlockOcclusion::NONE));

  //      nextBlock = &GETBLOCK(_blockIDData[wc + PADDED_OFFSETS::BACK]);
  //      faces[ZNEG] = ((nextBlock->caIndex != block.caIndex) && (nextBlock->occlude == BlockOcclusion::NONE));

  //      nextBlock = &GETBLOCK(_blockIDData[wc + PADDED_OFFSETS::RIGHT]);
  //      faces[XPOS] = ((nextBlock->caIndex != block.caIndex) && (nextBlock->occlude == BlockOcclusion::NONE));

  //      nextBlock = &GETBLOCK(_blockIDData[wc + PADDED_OFFSETS::FRONT]);
  //      faces[ZPOS] = ((nextBlock->caIndex != block.caIndex) && (nextBlock->occlude == BlockOcclusion::NONE));

  //      nextBlock = &GETBLOCK(_blockIDData[wc + PADDED_OFFSETS::BOTTOM]);
  //      faces[YNEG] = ((nextBlock->caIndex != block.caIndex) && (nextBlock->occlude == BlockOcclusion::NONE));
  //  }

  //  left = LEVEL(wc + PADDED_OFFSETS::LEFT);
  //  right = LEVEL(wc + PADDED_OFFSETS::RIGHT);
  //  back = LEVEL(wc + PADDED_OFFSETS::BACK);
  //  front = LEVEL(wc + PADDED_OFFSETS::FRONT);
  //  bottom = LEVEL(wc + PADDED_OFFSETS::BOTTOM);

  //  //Calculate the liquid levels

  //  //Back Left Vertex
  //  CALCULATE_LIQUID_VERTEX_HEIGHT(backLeftHeight, left, back, wc + PADDED_OFFSETS::BACK_LEFT);

  //  //Back Right Vertex
  //  CALCULATE_LIQUID_VERTEX_HEIGHT(backRightHeight, right, back, wc + PADDED_OFFSETS::BACK_RIGHT);

  //  //Front Right Vertex
  //  CALCULATE_LIQUID_VERTEX_HEIGHT(frontRightHeight, right, front, wc + PADDED_OFFSETS::FRONT_RIGHT);

  //  //Front Left Vertex
  //  CALCULATE_LIQUID_VERTEX_HEIGHT(frontLeftHeight, left, front, wc + PADDED_OFFSETS::FRONT_LEFT);

  //  //only occlude top if we are a full water block and our sides arent down at all
  //  if (liquidLevel == 1.0f && backRightHeight == 1.0f && backLeftHeight == 1.0f && frontLeftHeight == 1.0f && frontRightHeight == 1.0f) {
  //      nextBlock = &GETBLOCK(_blockIDData[wc + PADDED_OFFSETS::TOP]);
  //      faces[YPOS] = ((nextBlock->caIndex != block.caIndex) && (nextBlock->occlude == BlockOcclusion::NONE));
  //  } else {
  //      faces[YPOS] = true;
  //  }
  //  
  //  //Compute alpha
  //  if (bottom == maxLevel) {
  //      backRightAlpha = backLeftAlpha = frontRightAlpha = frontLeftAlpha = MAX_ALPHA;
  //  } else {
  //      backRightAlpha = (ui8)(backRightHeight * ALPHA_RANGE + MIN_ALPHA);
  //      backLeftAlpha = (ui8)(backLeftHeight * ALPHA_RANGE + MIN_ALPHA);
  //      frontRightAlpha = (ui8)(frontRightHeight * ALPHA_RANGE + MIN_ALPHA);
  //      frontLeftAlpha = (ui8)(frontLeftHeight * ALPHA_RANGE + MIN_ALPHA);
  //  }

  //  //Add vertices for the faces
  //  if (faces[YNEG]){

  //      VoxelMesher::makeLiquidFace(_waterVboVerts, mi.liquidIndex, uOff, vOff, lampLight, sunlight, color, textureUnit);
  //      
  //      _waterVboVerts[mi.liquidIndex].position[0] = x + VoxelMesher::liquidVertices[48];
  //      _waterVboVerts[mi.liquidIndex].position[1] = y + VoxelMesher::liquidVertices[49] - fallingReduction;
  //      _waterVboVerts[mi.liquidIndex].position[2] = z + VoxelMesher::liquidVertices[50];
  //      _waterVboVerts[mi.liquidIndex + 1].position[0] = x + VoxelMesher::liquidVertices[51];
  //      _waterVboVerts[mi.liquidIndex + 1].position[1] = y + VoxelMesher::liquidVertices[52] - fallingReduction;
  //      _waterVboVerts[mi.liquidIndex + 1].position[2] = z + VoxelMesher::liquidVertices[53];
  //      _waterVboVerts[mi.liquidIndex + 2].position[0] = x + VoxelMesher::liquidVertices[54];
  //      _waterVboVerts[mi.liquidIndex + 2].position[1] = y + VoxelMesher::liquidVertices[55] - fallingReduction;
  //      _waterVboVerts[mi.liquidIndex + 2].position[2] = z + VoxelMesher::liquidVertices[56];
  //      _waterVboVerts[mi.liquidIndex + 3].position[0] = x + VoxelMesher::liquidVertices[57];
  //      _waterVboVerts[mi.liquidIndex + 3].position[1] = y + VoxelMesher::liquidVertices[58] - fallingReduction;
  //      _waterVboVerts[mi.liquidIndex + 3].position[2] = z + VoxelMesher::liquidVertices[59];

  //      //set alpha
  //      _waterVboVerts[mi.liquidIndex].color.a = backLeftAlpha;
  //      _waterVboVerts[mi.liquidIndex + 1].color.a = backRightAlpha;
  //      _waterVboVerts[mi.liquidIndex + 2].color.a = frontRightAlpha;
  //      _waterVboVerts[mi.liquidIndex + 3].color.a = backLeftAlpha;

  //      mi.liquidIndex += 4;
  //  }

  //  if (faces[ZPOS]){

  //      VoxelMesher::makeLiquidFace(_waterVboVerts, mi.liquidIndex, uOff, vOff, lampLight, sunlight, color, textureUnit);

  //      _waterVboVerts[mi.liquidIndex].position[0] = x + VoxelMesher::liquidVertices[0];
  //      _waterVboVerts[mi.liquidIndex].position[1] = y + frontLeftHeight;
  //      _waterVboVerts[mi.liquidIndex].position[2] = z + VoxelMesher::liquidVertices[2];
  //      _waterVboVerts[mi.liquidIndex + 1].position[0] = x + VoxelMesher::liquidVertices[3];
  //      _waterVboVerts[mi.liquidIndex + 1].position[1] = y + VoxelMesher::liquidVertices[4] - fallingReduction;
  //      _waterVboVerts[mi.liquidIndex + 1].position[2] = z + VoxelMesher::liquidVertices[5];
  //      _waterVboVerts[mi.liquidIndex + 2].position[0] = x + VoxelMesher::liquidVertices[6];
  //      _waterVboVerts[mi.liquidIndex + 2].position[1] = y + VoxelMesher::liquidVertices[7] - fallingReduction;
  //      _waterVboVerts[mi.liquidIndex + 2].position[2] = z + VoxelMesher::liquidVertices[8];
  //      _waterVboVerts[mi.liquidIndex + 3].position[0] = x + VoxelMesher::liquidVertices[9];
  //      _waterVboVerts[mi.liquidIndex + 3].position[1] = y + frontRightHeight;
  //      _waterVboVerts[mi.liquidIndex + 3].position[2] = z + VoxelMesher::liquidVertices[11];

  //      _waterVboVerts[mi.liquidIndex].color.a = frontLeftAlpha;
  //      _waterVboVerts[mi.liquidIndex + 1].color.a = frontLeftAlpha;
  //      _waterVboVerts[mi.liquidIndex + 2].color.a = frontRightAlpha;
  //      _waterVboVerts[mi.liquidIndex + 3].color.a = frontRightAlpha;

  //      mi.liquidIndex += 4;
  //  }

  //  if (faces[YPOS]){

  //      VoxelMesher::makeLiquidFace(_waterVboVerts, mi.liquidIndex, uOff, vOff, lampLight, sunlight, color, textureUnit);

  //      _waterVboVerts.resize(_waterVboVerts.size() + 4);
  //      _waterVboVerts[mi.liquidIndex].position[0] = x + VoxelMesher::liquidVertices[24];
  //      _waterVboVerts[mi.liquidIndex].position[1] = y + backLeftHeight;
  //      _waterVboVerts[mi.liquidIndex].position[2] = z + VoxelMesher::liquidVertices[26];
  //      _waterVboVerts[mi.liquidIndex + 1].position[0] = x + VoxelMesher::liquidVertices[27];
  //      _waterVboVerts[mi.liquidIndex + 1].position[1] = y + frontLeftHeight;
  //      _waterVboVerts[mi.liquidIndex + 1].position[2] = z + VoxelMesher::liquidVertices[29];
  //      _waterVboVerts[mi.liquidIndex + 2].position[0] = x + VoxelMesher::liquidVertices[30];
  //      _waterVboVerts[mi.liquidIndex + 2].position[1] = y + frontRightHeight;
  //      _waterVboVerts[mi.liquidIndex + 2].position[2] = z + VoxelMesher::liquidVertices[32];
  //      _waterVboVerts[mi.liquidIndex + 3].position[0] = x + VoxelMesher::liquidVertices[33];
  //      _waterVboVerts[mi.liquidIndex + 3].position[1] = y + backRightHeight;
  //      _waterVboVerts[mi.liquidIndex + 3].position[2] = z + VoxelMesher::liquidVertices[35];

  //      _waterVboVerts[mi.liquidIndex].color.a = backLeftAlpha;
  //      _waterVboVerts[mi.liquidIndex + 1].color.a = frontLeftAlpha;
  //      _waterVboVerts[mi.liquidIndex + 2].color.a = frontRightAlpha;
  //      _waterVboVerts[mi.liquidIndex + 3].color.a = backRightAlpha;

  //      mi.liquidIndex += 4;
  //  }

  //  if (faces[ZNEG]){

  //      VoxelMesher::makeLiquidFace(_waterVboVerts, mi.liquidIndex, uOff, vOff, lampLight, sunlight, color, textureUnit);

  //      _waterVboVerts[mi.liquidIndex].position[0] = x + VoxelMesher::liquidVertices[60];
  //      _waterVboVerts[mi.liquidIndex].position[1] = y + backRightHeight;
  //      _waterVboVerts[mi.liquidIndex].position[2] = z + VoxelMesher::liquidVertices[62];
  //      _waterVboVerts[mi.liquidIndex + 1].position[0] = x + VoxelMesher::liquidVertices[63];
  //      _waterVboVerts[mi.liquidIndex + 1].position[1] = y + VoxelMesher::liquidVertices[64] - fallingReduction;
  //      _waterVboVerts[mi.liquidIndex + 1].position[2] = z + VoxelMesher::liquidVertices[65];
  //      _waterVboVerts[mi.liquidIndex + 2].position[0] = x + VoxelMesher::liquidVertices[66];
  //      _waterVboVerts[mi.liquidIndex + 2].position[1] = y + VoxelMesher::liquidVertices[67] - fallingReduction;
  //      _waterVboVerts[mi.liquidIndex + 2].position[2] = z + VoxelMesher::liquidVertices[68];
  //      _waterVboVerts[mi.liquidIndex + 3].position[0] = x + VoxelMesher::liquidVertices[69];
  //      _waterVboVerts[mi.liquidIndex + 3].position[1] = y + backLeftHeight;
  //      _waterVboVerts[mi.liquidIndex + 3].position[2] = z + VoxelMesher::liquidVertices[71];

  //      _waterVboVerts[mi.liquidIndex].color.a = backRightAlpha;
  //      _waterVboVerts[mi.liquidIndex + 1].color.a = backRightAlpha;
  //      _waterVboVerts[mi.liquidIndex + 2].color.a = backLeftAlpha;
  //      _waterVboVerts[mi.liquidIndex + 3].color.a = backLeftAlpha;

  //      mi.liquidIndex += 4;
  //  }
  //  if (faces[XPOS]){

  //      VoxelMesher::makeLiquidFace(_waterVboVerts, mi.liquidIndex, uOff, vOff, lampLight, sunlight, color, textureUnit);

  //      _waterVboVerts[mi.liquidIndex].position.x = x + VoxelMesher::liquidVertices[12];
  //      _waterVboVerts[mi.liquidIndex].position.y = y + frontRightHeight;
  //      _waterVboVerts[mi.liquidIndex].position.z = z + VoxelMesher::liquidVertices[14];
  //      _waterVboVerts[mi.liquidIndex + 1].position.x = x + VoxelMesher::liquidVertices[15];
  //      _waterVboVerts[mi.liquidIndex + 1].position.y = y + VoxelMesher::liquidVertices[16] - fallingReduction;
  //      _waterVboVerts[mi.liquidIndex + 1].position.z = z + VoxelMesher::liquidVertices[17];
  //      _waterVboVerts[mi.liquidIndex + 2].position.x = x + VoxelMesher::liquidVertices[18];
  //      _waterVboVerts[mi.liquidIndex + 2].position.y = y + VoxelMesher::liquidVertices[19] - fallingReduction;
  //      _waterVboVerts[mi.liquidIndex + 2].position.z = z + VoxelMesher::liquidVertices[20];
  //      _waterVboVerts[mi.liquidIndex + 3].position.x = x + VoxelMesher::liquidVertices[21];
  //      _waterVboVerts[mi.liquidIndex + 3].position.y = y + backRightHeight;
  //      _waterVboVerts[mi.liquidIndex + 3].position.z = z + VoxelMesher::liquidVertices[23];

  //      _waterVboVerts[mi.liquidIndex].color.a = frontRightAlpha;
  //      _waterVboVerts[mi.liquidIndex + 1].color.a = frontRightAlpha;
  //      _waterVboVerts[mi.liquidIndex + 2].color.a = backRightAlpha;
  //      _waterVboVerts[mi.liquidIndex + 3].color.a = backRightAlpha;

  //      mi.liquidIndex += 4;
  //  }
  //  if (faces[XNEG]){

  //      VoxelMesher::makeLiquidFace(_waterVboVerts, mi.liquidIndex, uOff, vOff, lampLight, sunlight, color, textureUnit);

  //      _waterVboVerts[mi.liquidIndex].position[0] = x + VoxelMesher::liquidVertices[36];
  //      _waterVboVerts[mi.liquidIndex].position[1] = y + backLeftHeight;
  //      _waterVboVerts[mi.liquidIndex].position[2] = z + VoxelMesher::liquidVertices[38];
  //      _waterVboVerts[mi.liquidIndex + 1].position[0] = x + VoxelMesher::liquidVertices[39];
  //      _waterVboVerts[mi.liquidIndex + 1].position[1] = y + VoxelMesher::liquidVertices[40] - fallingReduction;
  //      _waterVboVerts[mi.liquidIndex + 1].position[2] = z + VoxelMesher::liquidVertices[41];
  //      _waterVboVerts[mi.liquidIndex + 2].position[0] = x + VoxelMesher::liquidVertices[42];
  //      _waterVboVerts[mi.liquidIndex + 2].position[1] = y + VoxelMesher::liquidVertices[43] - fallingReduction;
  //      _waterVboVerts[mi.liquidIndex + 2].position[2] = z + VoxelMesher::liquidVertices[44];
  //      _waterVboVerts[mi.liquidIndex + 3].position[0] = x + VoxelMesher::liquidVertices[45];
  //      _waterVboVerts[mi.liquidIndex + 3].position[1] = y + frontLeftHeight;
  //      _waterVboVerts[mi.liquidIndex + 3].position[2] = z + VoxelMesher::liquidVertices[47];

  //      _waterVboVerts[mi.liquidIndex].color.a = backLeftAlpha;
  //      _waterVboVerts[mi.liquidIndex + 1].color.a = backLeftAlpha;
  //      _waterVboVerts[mi.liquidIndex + 2].color.a = frontLeftAlpha;
  //      _waterVboVerts[mi.liquidIndex + 3].color.a = frontLeftAlpha;

  //      mi.liquidIndex += 4;
  //  }
}

int ChunkMesher::getLiquidLevel(int blockIndex, const Block& block) {
    int val = GETBLOCKID(m_blockData[blockIndex]); // Get block ID
    val = val - block.liquidStartID;
    if (val < 0) return 0;
    if (val > block.liquidLevels) return 0;
    return val;
}

bool ChunkMesher::createChunkMesh(RenderTask *renderTask)
{

    m_highestY = 0;
    m_lowestY = 256;
    m_highestX = 0;
    m_lowestX = 256;
    m_highestZ = 0;
    m_lowestZ = 256;

    // Clear quad indices
    memset(m_quadIndices, 0, sizeof(m_quadIndices));

    for (int i = 0; i < 6; i++) {
        m_quads[i].clear();
    }

    int waveEffect;
    // CONST?
    Chunk* chunk = renderTask->chunk;

    // Here?
    _waterVboVerts.clear();
    _transparentVerts.clear();
    _cutoutVerts.clear();

    //store the render task so we can pass it to functions
  //  mi.task = renderTask;
   // mi.chunkGridData = chunk->gridData;
   // mi.position = chunk->getVoxelPosition().pos;


    //create a new chunk mesh data container
    if (chunkMeshData != NULL){
        pError("Tried to create mesh with in use chunkMeshData!");
        return 0;
    }

    //Stores the data for a chunk mesh
    // TODO(Ben): new is bad mkay
    chunkMeshData = new ChunkMeshData(renderTask);
    /*
        mi.blockIDData = m_blockData;
        mi.lampLightData = m_lampData;
        mi.sunlightData = m_sunData;
        mi.tertiaryData = m_tertiaryData;*/

    m_dataLayer = PADDED_CHUNK_LAYER;
    m_dataWidth = PADDED_CHUNK_WIDTH;
    m_dataSize = PADDED_CHUNK_SIZE;

    // Init the mesh info
    // Redundant
    //mi.init(m_blocks, m_dataWidth, m_dataLayer);

    // Loop through blocks
    for (by = 0; by < m_dataWidth-2; by++) {
        for (bz = 0; bz < m_dataWidth-2; bz++){
            for (bx = 0; bx < m_dataWidth-2; bx++){
                // Get data for this voxel
                // TODO(Ben): Could optimize out -1
                m_blockIndex = (by + 1) * m_dataLayer + (bz + 1) * m_dataWidth + (bx + 1);
                m_blockID = m_blockData[m_blockIndex];
                if (m_blockID == 0) continue; // Skip air blocks
                m_heightData = &chunkGridData->heightData[(bz - 1) * CHUNK_WIDTH + bx - 1];
                m_block = &m_blocks->operator[](m_blockID);
                // TODO(Ben) Don't think bx needs to be member
                m_voxelPosOffset = ui8v3(bx * 7, by * 7, bz * 7);

                switch (m_block->meshType) {
                    case MeshType::BLOCK:
                        addBlock();
                        break;
                    case MeshType::LEAVES:
                    case MeshType::CROSSFLORA:
                    case MeshType::FLORA:
                        addFlora();
                        break;
                    default:
                        //No mesh, do nothing
                        break;
                }
            }
        }
    }

    ChunkMeshRenderData& renderData = chunkMeshData->chunkMeshRenderData;

    // Get quad buffer to fill
    std::vector<VoxelQuad>& quads = chunkMeshData->opaqueQuads;
    // Start by swapping with the X_NEG to reduce copying
    quads.swap(m_quads[0]);
    int nxSize = quads.size();
    int offset = nxSize;
    quads.resize(quads.size() + m_quads[1].size() + m_quads[2].size() +
                 m_quads[3].size() + m_quads[4].size() + m_quads[5].size());
    // Copy the data
    // TODO(Ben): Could construct in place and not need ANY copying with 6 iterations.
    for (int i = 1; i < 6; i++) {
        memcpy(quads.data() + offset, m_quads[i].data(), m_quads[i].size() * sizeof(VoxelQuad));
        offset += m_quads[i].size();
    }

    m_highestY /= 7;
    m_lowestY /= 7;
    m_highestX /= 7;
    m_lowestX /= 7;
    m_highestZ /= 7;
    m_lowestZ /= 7;

#define INDICES_PER_QUAD 6

    if (quads.size()) {
        renderData.nxVboOff = 0;
        renderData.nxVboSize = nxSize * INDICES_PER_QUAD;
        renderData.pxVboOff = renderData.nxVboSize;
        renderData.pxVboSize = m_quads[1].size() * INDICES_PER_QUAD;
        renderData.nyVboOff = renderData.pxVboOff + renderData.pxVboSize;
        renderData.nyVboSize = m_quads[2].size() * INDICES_PER_QUAD;
        renderData.pyVboOff = renderData.nyVboOff + renderData.nyVboSize;
        renderData.pyVboSize = m_quads[3].size() * INDICES_PER_QUAD;
        renderData.nzVboOff = renderData.pyVboOff + renderData.pyVboSize;
        renderData.nzVboSize = m_quads[4].size() * INDICES_PER_QUAD;
        renderData.pzVboOff = renderData.nzVboOff + renderData.nzVboSize;
        renderData.pzVboSize = m_quads[5].size() * INDICES_PER_QUAD;
        renderData.indexSize = quads.size() * INDICES_PER_QUAD;

        // Redundant
        renderData.highestX = m_highestX;
        renderData.lowestX = m_lowestX;
        renderData.highestY = m_highestY;
        renderData.lowestY = m_lowestY;
        renderData.highestZ = m_highestZ;
        renderData.lowestZ = m_lowestZ;
    }

 //   int indice = (index / 4) * 6;

 //   ChunkMeshRenderData& meshInfo = chunkMeshData->chunkMeshRenderData;

 //   //add all vertices to the vbo
 //   if (chunkMeshData->vertices.size() || chunkMeshData->transVertices.size() || chunkMeshData->cutoutVertices.size()) {
 //       meshInfo.indexSize = (chunkMeshData->vertices.size() * 6) / 4;

	//	//now let the sizes represent indice sizes
	//	meshInfo.pyVboOff = pyVboOff;
	//	meshInfo.pyVboSize = (mi.pyVboSize / 4) * 6;
	//	meshInfo.nyVboOff = nyVboOff;
	//	meshInfo.nyVboSize = (mi.nyVboSize / 4) * 6;
	//	meshInfo.pxVboOff = pxVboOff;
	//	meshInfo.pxVboSize = (mi.pxVboSize / 4) * 6;
	//	meshInfo.nxVboOff = nxVboOff;
	//	meshInfo.nxVboSize = (mi.nxVboSize / 4) * 6;
	//	meshInfo.pzVboOff = pzVboOff;
	//	meshInfo.pzVboSize = (mi.pzVboSize / 4) * 6;
	//	meshInfo.nzVboOff = nzVboOff;
	//	meshInfo.nzVboSize = (mi.nzVboSize / 4) * 6;

 //       meshInfo.transVboSize = (mi.transparentIndex / 4) * 6;
 //       meshInfo.cutoutVboSize = (mi.cutoutIndex / 4) * 6;
 //     
	//	meshInfo.highestY = highestY;
	//	meshInfo.lowestY = lowestY;
	//	meshInfo.highestX = highestX;
	//	meshInfo.lowestX = lowestX;
	//	meshInfo.highestZ = highestZ;
	//	meshInfo.lowestZ = lowestZ;
	//}

	//if (mi.liquidIndex){
 //       meshInfo.waterIndexSize = (mi.liquidIndex * 6) / 4;
 //       chunkMeshData->waterVertices.swap(_waterVboVerts);
	//}

	return 0;
}

bool ChunkMesher::createOnlyWaterMesh(RenderTask *renderTask)
{
	if (chunkMeshData != NULL){
		pError("Tried to create mesh with in use chunkMeshData!");
		return 0;
	}
    chunkMeshData = new ChunkMeshData(renderTask);

    _waterVboVerts.clear();
    MesherInfo mi = {};

    mi.task = renderTask;

    for (int i = 0; i < wSize; i++) {
        mi.wc = m_wvec[i];
        mi.btype = GETBLOCKID(m_blockData[mi.wc]);
        mi.x = (mi.wc % PADDED_CHUNK_WIDTH) - 1;
        mi.y = (mi.wc / PADDED_CHUNK_LAYER) - 1;
        mi.z = ((mi.wc % PADDED_CHUNK_LAYER) / PADDED_CHUNK_WIDTH) - 1;
      
        addLiquid(mi);
    }


	if (mi.liquidIndex){
        chunkMeshData->chunkMeshRenderData.waterIndexSize = (mi.liquidIndex * 6) / 4;
		chunkMeshData->waterVertices.swap(_waterVboVerts);
	}
    
    return 0;
}

void ChunkMesher::freeBuffers()
{
    //free memory
    //std::vector <BlockVertex>().swap(_vboVerts);

    //These dont get too big so it might be ok to not free them?
    /*vector <Vertex>().swap(waterVboVerts);
    vector<Vertex>().swap(finalTopVerts);
    vector<Vertex>().swap(finalLeftVerts);
    vector<Vertex>().swap(finalRightVerts);
    vector<Vertex>().swap(finalFrontVerts);
    vector<Vertex>().swap(finalBackVerts);
    vector<Vertex>().swap(finalBottomVerts);
    vector<Vertex>().swap(finalNbVerts);*/
}
