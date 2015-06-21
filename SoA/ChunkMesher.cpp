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

//Calculates the smooth lighting based on accumulated light and num adjacent blocks.
//Returns it as a GLubyte for vertex data
GLubyte ChunkMesher::calculateSmoothLighting(int accumulatedLight, int numAdjacentBlocks) {
    return (GLubyte)(255.0f * (LIGHT_OFFSET + pow(LIGHT_MULT, MAXLIGHT - ((float)(accumulatedLight) / (4 - numAdjacentBlocks)))));
}

void ChunkMesher::calculateLampColor(ColorRGB8& dst, ui16 src0, ui16 src1, ui16 src2, ui16 src3, ui8 numAdj) {
    #define GETRED(a) ((a & LAMP_RED_MASK) >> LAMP_RED_SHIFT)
    #define GETGREEN(a) ((a & LAMP_GREEN_MASK) >> LAMP_GREEN_SHIFT)
    #define GETBLUE(a) (a & LAMP_BLUE_MASK)
    dst.r = calculateSmoothLighting(GETRED(src0) + GETRED(src1) + GETRED(src2) + GETRED(src3), numAdj);
    dst.g = calculateSmoothLighting(GETGREEN(src0) + GETGREEN(src1) + GETGREEN(src2) + GETGREEN(src3), numAdj);
    dst.b = calculateSmoothLighting(GETBLUE(src0) + GETBLUE(src1) + GETBLUE(src2) + GETBLUE(src3), numAdj);
}

void ChunkMesher::calculateFaceLight(BlockVertex* face, int blockIndex, int upOffset, int frontOffset, int rightOffset, f32 ambientOcclusion[]) {
    
    // Ambient occlusion factor
#define OCCLUSION_FACTOR 0.2f;
    // Helper macro
#define CALCULATE_VERTEX(v, s1, s2) \
    nearOccluders = (int)(GETBLOCK(_blockIDData[blockIndex]).occlude != BlockOcclusion::NONE) + \
    (int)(GETBLOCK(_blockIDData[blockIndex s1 frontOffset]).occlude != BlockOcclusion::NONE) + \
    (int)(GETBLOCK(_blockIDData[blockIndex s2 rightOffset]).occlude != BlockOcclusion::NONE) + \
    (int)(GETBLOCK(_blockIDData[blockIndex s1 frontOffset s2 rightOffset]).occlude != BlockOcclusion::NONE); \
    ambientOcclusion[v] = 1.0f - nearOccluders * OCCLUSION_FACTOR; \
    calculateLampColor(face[v].lampColor, _lampLightData[blockIndex], \
                       _lampLightData[blockIndex s1 frontOffset], \
                       _lampLightData[blockIndex s2 rightOffset], \
                       _lampLightData[blockIndex s1 frontOffset s2 rightOffset], \
                       nearOccluders); \
    face[v].sunlight = calculateSmoothLighting(_sunlightData[blockIndex] + \
                                               _sunlightData[blockIndex s1 frontOffset] + \
                                               _sunlightData[blockIndex s2 rightOffset] + \
                                               _sunlightData[blockIndex s1 frontOffset s2 rightOffset], \
                                               nearOccluders);

    // Move the block index upwards
    blockIndex += upOffset;
    int nearOccluders; ///< For ambient occlusion

    // Vertex 0
    CALCULATE_VERTEX(0, -, -)

    // Vertex 1
    CALCULATE_VERTEX(1, +, -)
   
    // Vertex 2
    CALCULATE_VERTEX(2, +, +)

    // Vertex 3
    CALCULATE_VERTEX(3, -, +)

}

void ChunkMesher::addBlock()
{
    ColorRGB8 color, overlayColor;
    int textureIndex, overlayTextureIndex;

    GLfloat ambientOcclusion[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    ui8 sunLight = m_sunData[m_blockIndex];
    // This is pretty ugly
    ColorRGB8 lampLight((m_lampData[m_blockIndex] & LAMP_RED_MASK) >> LAMP_RED_SHIFT,
                        (m_lampData[m_blockIndex] & LAMP_GREEN_MASK) >> LAMP_GREEN_SHIFT,
                        m_lampData[m_blockIndex] & LAMP_BLUE_MASK);

    // So is this
    sunLight = (ui8)(255.0f*(LIGHT_OFFSET + pow(LIGHT_MULT, MAXLIGHT - sunLight)));
    lampLight.r = (ui8)(255.0f*(LIGHT_OFFSET + pow(LIGHT_MULT, MAXLIGHT - lampLight.r)));
    lampLight.g = (ui8)(255.0f*(LIGHT_OFFSET + pow(LIGHT_MULT, MAXLIGHT - lampLight.g)));
    lampLight.b = (ui8)(255.0f*(LIGHT_OFFSET + pow(LIGHT_MULT, MAXLIGHT - lampLight.b)));

    //get bit flags (needs to be changed) -Ben
    //GLuint flags = GETFLAGS(mi.blockIDData[mi.wc]);

    // Check the faces
    // Left
    if (shouldRenderFace(-1)) {

    }
    // Right
    if (shouldRenderFace(1)) {

    }
    // Bottom
    if (shouldRenderFace(-PADDED_CHUNK_LAYER)) {

    }
    // Top
    if (shouldRenderFace(PADDED_CHUNK_LAYER)) {

    }
    // Back
    if (shouldRenderFace(-PADDED_CHUNK_WIDTH)) {

    }
    // Front
    if (shouldRenderFace(PADDED_CHUNK_WIDTH)) {

    }

    if (faces[ZPOS]){ //0 1 2 3
        //Get the color of the block
        (*m_blocks)[btype].GetBlockColor(color, overlayColor, flags, mi.temperature, mi.rainfall, block.textures[2]);
        
        //We will offset the texture index based on the texture method
        textureIndex = block.textures[2]->base.getBlockTextureIndex(mi.pzBaseMethodParams, color);
        overlayTextureIndex = block.textures[2]->overlay.getBlockTextureIndex(mi.pzOverlayMethodParams, overlayColor);

        if (block.textures[2]->base.transparency) {
            _transparentVerts.resize(_transparentVerts.size() + 4);
            VoxelMesher::makeTransparentFace(&(_transparentVerts[0]), VoxelMesher::cubeVertices, VoxelMesher::cubeNormals, 0, block.waveEffect, i32v3(mi.x, mi.y, mi.z), mi.transparentIndex, textureIndex, overlayTextureIndex, color, overlayColor, sunLight, lampLight, block.textures[2]);
            mi.transparentIndex += 4;

            chunkMeshData->addTransQuad(i8v3(mi.x2 + 1, mi.y2 + 1, mi.z2 + 2));

            mi.mergeFront = false;
        } else {
            //Set up the light data using smooth lighting
            calculateFaceLight(&_frontVerts[mi.frontIndex], wc, PADDED_CHUNK_WIDTH, -PADDED_CHUNK_LAYER, 1, ambientOcclusion);

            //Set up most of the vertex data for a face
            VoxelMesher::makeCubeFace(_frontVerts, CUBE_FACE_0_VERTEX_OFFSET, (int)block.waveEffect, glm::ivec3(mi.nx, mi.ny, mi.nz), mi.frontIndex, textureIndex, overlayTextureIndex, color, overlayColor, ambientOcclusion, block.textures[2]);

            //to check for a +x merge, we check that the vertices aligned in the direction of stretch are equal
            if (mi.mergeFront && mi.pbtype == btype &&
                CompareVertices(_frontVerts[mi.pfrontIndex], _frontVerts[mi.pfrontIndex + 3]) && CompareVertices(_frontVerts[mi.pfrontIndex + 3], _frontVerts[mi.frontIndex]) && CompareVertices(_frontVerts[mi.frontIndex], _frontVerts[mi.frontIndex + 3]) &&//-z vertices
                CompareVertices(_frontVerts[mi.pfrontIndex + 1], _frontVerts[mi.pfrontIndex + 2]) && CompareVertices(_frontVerts[mi.pfrontIndex + 2], _frontVerts[mi.frontIndex + 1]) && CompareVertices(_frontVerts[mi.frontIndex + 1], _frontVerts[mi.frontIndex + 2])){ //+z vertices

                _frontVerts[mi.pfrontIndex + 2].position.x += 7;
                _frontVerts[mi.pfrontIndex + 3].position.x += 7;
                _frontVerts[mi.pfrontIndex + 2].tex[0]++;
                _frontVerts[mi.pfrontIndex + 3].tex[0]++;
            } else{
                mi.pfrontIndex = mi.frontIndex;
                mi.frontIndex += 4;
                mi.mergeFront = 1;
            }
        }
    }
    else{
        mi.mergeFront = 0;
    }

    if (faces[ZNEG]) {
        //Get the color of the block
        (*m_blocks)[btype].GetBlockColor(color, overlayColor, flags, mi.temperature, mi.rainfall, block.textures[5]);
       
        //We will offset the texture index based on the texture method

        textureIndex = block.textures[5]->base.getBlockTextureIndex(mi.nzBaseMethodParams, color);
        overlayTextureIndex = block.textures[5]->overlay.getBlockTextureIndex(mi.nzOverlayMethodParams, overlayColor);

        if (block.textures[5]->base.transparency) {
            _transparentVerts.resize(_transparentVerts.size() + 4);
            VoxelMesher::makeTransparentFace(&(_transparentVerts[0]), VoxelMesher::cubeVertices, VoxelMesher::cubeNormals, 60, block.waveEffect, i32v3(mi.x, mi.y, mi.z), mi.transparentIndex, textureIndex, overlayTextureIndex, color, overlayColor, sunLight, lampLight, block.textures[5]);
            mi.transparentIndex += 4;

            chunkMeshData->addTransQuad(i8v3(mi.x2 + 1, mi.y2 + 1, mi.z2));
            mi.mergeBack = false;
        } else {

            calculateFaceLight(&_backVerts[mi.backIndex], wc, -PADDED_CHUNK_WIDTH, -PADDED_CHUNK_LAYER, -1, ambientOcclusion);

            VoxelMesher::makeCubeFace(_backVerts, CUBE_FACE_5_VERTEX_OFFSET, (int)block.waveEffect, glm::ivec3(mi.nx, mi.ny, mi.nz), mi.backIndex, textureIndex, overlayTextureIndex, color, overlayColor, ambientOcclusion, block.textures[5]);

            //to check for a +x merge, we check that the vertices aligned in the direction of stretch are equal
            if (mi.mergeBack && mi.pbtype == btype &&
                CompareVertices(_backVerts[mi.pbackIndex], _backVerts[mi.pbackIndex + 3]) && CompareVertices(_backVerts[mi.pbackIndex + 3], _backVerts[mi.backIndex]) && CompareVertices(_backVerts[mi.backIndex], _backVerts[mi.backIndex + 3]) &&//-z vertices
                CompareVertices(_backVerts[mi.pbackIndex + 1], _backVerts[mi.pbackIndex + 2]) && CompareVertices(_backVerts[mi.pbackIndex + 2], _backVerts[mi.backIndex + 1]) && CompareVertices(_backVerts[mi.backIndex + 1], _backVerts[mi.backIndex + 2])){ //+z vertices

                _backVerts[mi.pbackIndex].position.x += 7;
                _backVerts[mi.pbackIndex + 1].position.x += 7;
                _backVerts[mi.pbackIndex].tex[0]--;
                _backVerts[mi.pbackIndex + 1].tex[0]--;
            } else{
                mi.pbackIndex = mi.backIndex;
                mi.backIndex += 4;
                mi.mergeBack = 1;
            }
        }
    } else {
        mi.mergeBack = 0;
    }

    if (faces[YPOS]){ //0 5 6 1                        
        (*m_blocks)[btype].GetBlockColor(color, overlayColor, flags, mi.temperature, mi.rainfall, block.textures[1]);

        //We will offset the texture index based on the texture method
        textureIndex = block.textures[1]->base.getBlockTextureIndex(mi.pyBaseMethodParams, color);
        overlayTextureIndex = block.textures[1]->overlay.getBlockTextureIndex(mi.pyOverlayMethodParams, overlayColor);

        if (block.textures[1]->base.transparency) {
            _transparentVerts.resize(_transparentVerts.size() + 4);
            VoxelMesher::makeTransparentFace(&(_transparentVerts[0]), VoxelMesher::cubeVertices, VoxelMesher::cubeNormals, 24, block.waveEffect, i32v3(mi.x, mi.y, mi.z), mi.transparentIndex, textureIndex, overlayTextureIndex, color, overlayColor, sunLight, lampLight, block.textures[1]);
            mi.transparentIndex += 4;

            chunkMeshData->addTransQuad(i8v3(mi.x2 + 1, mi.y2 + 2, mi.z2 + 1));

            mi.mergeUp = false;
        } else {
            calculateFaceLight(&m_topVerts[mi.topIndex], wc, PADDED_CHUNK_LAYER, PADDED_CHUNK_WIDTH, 1, ambientOcclusion);

            VoxelMesher::makeCubeFace(m_topVerts, CUBE_FACE_2_VERTEX_OFFSET, (int)block.waveEffect, glm::ivec3(mi.nx, mi.ny, mi.nz), mi.topIndex, textureIndex, overlayTextureIndex, color, overlayColor, ambientOcclusion, block.textures[1]);
            
            //to check for a +x merge, we check that the vertices aligned in the direction of stretch are equal
            if (mi.mergeUp && mi.pbtype == btype &&
                CompareVertices(m_topVerts[mi.pupIndex], m_topVerts[mi.pupIndex + 3]) && CompareVertices(m_topVerts[mi.pupIndex + 3], m_topVerts[mi.topIndex]) && CompareVertices(m_topVerts[mi.topIndex], m_topVerts[mi.topIndex + 3]) &&//-z vertices
                CompareVertices(m_topVerts[mi.pupIndex + 1], m_topVerts[mi.pupIndex + 2]) && CompareVertices(m_topVerts[mi.pupIndex + 2], m_topVerts[mi.topIndex + 1]) && CompareVertices(m_topVerts[mi.topIndex + 1], m_topVerts[mi.topIndex + 2])){ //+z vertices
                m_topVerts[mi.pupIndex + 2].position.x += 7;                    //change x
                m_topVerts[mi.pupIndex + 3].position.x += 7;
                m_topVerts[mi.pupIndex + 2].tex[0]++;
                m_topVerts[mi.pupIndex + 3].tex[0]++;
            } else{
                mi.pupIndex = mi.topIndex;
                mi.topIndex += 4;
                mi.mergeUp = true;
            }
        }
    }
    else{
        mi.mergeUp = false;
    }

    if (faces[YNEG]) {
        (*m_blocks)[btype].GetBlockColor(color, overlayColor, flags, mi.temperature, mi.rainfall, block.textures[4]);
       
        //We will offset the texture index based on the texture method
        textureIndex = block.textures[4]->base.getBlockTextureIndex(mi.nyBaseMethodParams, color);
        overlayTextureIndex = block.textures[4]->overlay.getBlockTextureIndex(mi.nyOverlayMethodParams, overlayColor);

        if (block.textures[4]->base.transparency) {
            _transparentVerts.resize(_transparentVerts.size() + 4);
            VoxelMesher::makeTransparentFace(&(_transparentVerts[0]), VoxelMesher::cubeVertices, VoxelMesher::cubeNormals, 48, block.waveEffect, i32v3(mi.x, mi.y, mi.z), mi.transparentIndex, textureIndex, overlayTextureIndex, color, overlayColor, sunLight, lampLight, block.textures[4]);
            mi.transparentIndex += 4;

            chunkMeshData->addTransQuad(i8v3(mi.x2 + 1, mi.y2, mi.z2 + 1));

            mi.mergeBot = false;
        } else {
            calculateFaceLight(&_bottomVerts[mi.botIndex], wc, -PADDED_CHUNK_LAYER, PADDED_CHUNK_WIDTH, -1, ambientOcclusion);

            VoxelMesher::makeCubeFace(_bottomVerts, CUBE_FACE_4_VERTEX_OFFSET, (int)block.waveEffect, glm::ivec3(mi.nx, mi.ny, mi.nz), mi.botIndex, textureIndex, overlayTextureIndex, color, overlayColor, ambientOcclusion, block.textures[4]);

            //to check for a +x merge, we check that the vertices aligned in the direction of stretch are equal
            if (mi.mergeBot && mi.pbtype == btype &&
                CompareVertices(_bottomVerts[mi.pbotIndex], _bottomVerts[mi.pbotIndex + 3]) && CompareVertices(_bottomVerts[mi.pbotIndex + 3], _bottomVerts[mi.botIndex]) && CompareVertices(_bottomVerts[mi.botIndex], _bottomVerts[mi.botIndex + 3]) &&//-z vertices
                CompareVertices(_bottomVerts[mi.pbotIndex + 1], _bottomVerts[mi.pbotIndex + 2]) && CompareVertices(_bottomVerts[mi.pbotIndex + 2], _bottomVerts[mi.botIndex + 1]) && CompareVertices(_bottomVerts[mi.botIndex + 1], _bottomVerts[mi.botIndex + 2])){ //+z vertices
                _bottomVerts[mi.pbotIndex].position.x += 7;                    //change x
                _bottomVerts[mi.pbotIndex + 1].position.x += 7;
                _bottomVerts[mi.pbotIndex].tex[0]--;
                _bottomVerts[mi.pbotIndex + 1].tex[0]--;
            } else {
                mi.pbotIndex = mi.botIndex;
                mi.botIndex += 4;
                mi.mergeBot = 1;
            }
        }
    } else{
        mi.mergeBot = 0;
    }

    if (faces[XPOS]) {
        (*m_blocks)[btype].GetBlockColor(color, overlayColor, flags, mi.temperature, mi.rainfall, block.textures[0]);
      
        //We will offset the texture index based on the texture method
        textureIndex = block.textures[0]->base.getBlockTextureIndex(mi.pxBaseMethodParams, color);
        overlayTextureIndex = block.textures[0]->overlay.getBlockTextureIndex(mi.pxOverlayMethodParams, overlayColor);


        if (block.textures[0]->base.transparency) {
            _transparentVerts.resize(_transparentVerts.size() + 4);
            VoxelMesher::makeTransparentFace(&(_transparentVerts[0]), VoxelMesher::cubeVertices, VoxelMesher::cubeNormals, 12, block.waveEffect, i32v3(mi.x, mi.y, mi.z), mi.transparentIndex, textureIndex, overlayTextureIndex, color, overlayColor, sunLight, lampLight, block.textures[0]);
            mi.transparentIndex += 4;

            chunkMeshData->addTransQuad(i8v3(mi.x2 + 2, mi.y2 + 1, mi.z2 + 1));
        } else {
            calculateFaceLight(&m_rightVerts[mi.rightIndex], wc, 1, -PADDED_CHUNK_LAYER, -PADDED_CHUNK_WIDTH, ambientOcclusion);

            VoxelMesher::makeCubeFace(m_rightVerts, CUBE_FACE_1_VERTEX_OFFSET, (int)block.waveEffect, glm::ivec3(mi.nx, mi.ny, mi.nz), mi.rightIndex, textureIndex, overlayTextureIndex, color, overlayColor, ambientOcclusion, block.textures[0]);

            mi.rightIndex += 4;
        }
    }

    if (faces[XNEG]) {
        (*m_blocks)[btype].GetBlockColor(color, overlayColor, flags, mi.temperature, mi.rainfall, block.textures[3]);

        //We will offset the texture index based on the texture method
        textureIndex = block.textures[3]->base.getBlockTextureIndex(mi.nxBaseMethodParams, color);
        overlayTextureIndex = block.textures[3]->overlay.getBlockTextureIndex(mi.nxOverlayMethodParams, overlayColor);

        if (block.textures[3]->base.transparency) {
            _transparentVerts.resize(_transparentVerts.size() + 4);
            VoxelMesher::makeTransparentFace(&(_transparentVerts[0]), VoxelMesher::cubeVertices, VoxelMesher::cubeNormals, 36, block.waveEffect, i32v3(mi.x, mi.y, mi.z), mi.transparentIndex, textureIndex, overlayTextureIndex, color, overlayColor, sunLight, lampLight, block.textures[3]);
            mi.transparentIndex += 4;

            chunkMeshData->addTransQuad(i8v3(mi.x2, mi.y2 + 1, mi.z2 + 1));
        } else {
           
            calculateFaceLight(&m_leftVerts[mi.leftIndex], wc, -1, -PADDED_CHUNK_LAYER, PADDED_CHUNK_WIDTH, ambientOcclusion);

            VoxelMesher::makeCubeFace(m_leftVerts, CUBE_FACE_3_VERTEX_OFFSET, (int)block.waveEffect, glm::ivec3(mi.nx, mi.ny, mi.nz), mi.leftIndex, textureIndex, overlayTextureIndex, color, overlayColor, ambientOcclusion, block.textures[3]);

            mi.leftIndex += 4;
        }
    }
}

//adds a flora mesh
void ChunkMesher::addFlora() {

    const Block &block = (*m_blocks)[mi.btype];
    mi.currentBlock = &block;

    ColorRGB8 color, overlayColor;
    const int wc = mi.wc;
    const int btype = mi.btype;

    //Biome *biome = mi.chunkGridData->heightData[mi.nz*CHUNK_WIDTH + mi.nx].biome;
    int temperature = mi.chunkGridData->heightData[mi.nz*CHUNK_WIDTH + mi.nx].temperature;
    int rainfall = mi.chunkGridData->heightData[mi.nz*CHUNK_WIDTH + mi.nx].rainfall;

    GLuint flags = GETFLAGS(m_blockData[mi.wc]);

    ui8 sunLight = m_sunData[wc];
    ColorRGB8 lampLight((m_lampData[wc] & LAMP_RED_MASK) >> LAMP_RED_SHIFT,
                        (m_lampData[wc] & LAMP_GREEN_MASK) >> LAMP_GREEN_SHIFT,
                        m_lampData[wc] & LAMP_BLUE_MASK);

    sunLight = (ui8)(255.0f*(LIGHT_OFFSET + pow(LIGHT_MULT, MAXLIGHT - sunLight)));
    lampLight.r = (ui8)(255.0f*(LIGHT_OFFSET + pow(LIGHT_MULT, MAXLIGHT - lampLight.r)));
    lampLight.g = (ui8)(255.0f*(LIGHT_OFFSET + pow(LIGHT_MULT, MAXLIGHT - lampLight.g)));
    lampLight.b = (ui8)(255.0f*(LIGHT_OFFSET + pow(LIGHT_MULT, MAXLIGHT - lampLight.b)));

    (*m_blocks)[btype].GetBlockColor(color, overlayColor, flags, temperature, rainfall, block.textures[0]);

    //We will offset the texture index based on the texture method
    i32 textureIndex = block.textures[0]->base.getBlockTextureIndex(mi.pxBaseMethodParams, color);
    i32 overlayTextureIndex = block.textures[0]->overlay.getBlockTextureIndex(mi.pxOverlayMethodParams, overlayColor);

    int r;

    switch (mi.meshType){
        case MeshType::LEAVES:

            break;
        case MeshType::CROSSFLORA:
            //Generate a random number between 0 and 3 inclusive
            r = std::bind(std::uniform_int_distribution<int>(0, 1), std::mt19937(/*getPositionSeed(mi.nx, mi.nz)*/))();

            _cutoutVerts.resize(_cutoutVerts.size() + 8);
            VoxelMesher::makeFloraFace(&(_cutoutVerts[0]), VoxelMesher::crossFloraVertices[r], VoxelMesher::floraNormals, 0, block.waveEffect, i32v3(mi.nx, mi.ny, mi.nz), mi.cutoutIndex, textureIndex, overlayTextureIndex, color, overlayColor, sunLight, lampLight, block.textures[0]);
            mi.cutoutIndex += 4;
            VoxelMesher::makeFloraFace(&(_cutoutVerts[0]), VoxelMesher::crossFloraVertices[r], VoxelMesher::floraNormals, 12, block.waveEffect, i32v3(mi.nx, mi.ny, mi.nz), mi.cutoutIndex, textureIndex, overlayTextureIndex, color, overlayColor, sunLight, lampLight, block.textures[0]);
            mi.cutoutIndex += 4;
            break;
        case MeshType::FLORA:
            //Generate a random number between 0 and 3 inclusive
            r = std::bind(std::uniform_int_distribution<int>(0, 3), std::mt19937(/*getPositionSeed(mi.nx, mi.nz)*/))();

            _cutoutVerts.resize(_cutoutVerts.size() + 12);
            VoxelMesher::makeFloraFace(&(_cutoutVerts[0]), VoxelMesher::floraVertices[r], VoxelMesher::floraNormals, 0, block.waveEffect, i32v3(mi.nx, mi.ny, mi.nz), mi.cutoutIndex, textureIndex, overlayTextureIndex, color, overlayColor, sunLight, lampLight, block.textures[0]);
            mi.cutoutIndex += 4;
            VoxelMesher::makeFloraFace(&(_cutoutVerts[0]), VoxelMesher::floraVertices[r], VoxelMesher::floraNormals, 12, block.waveEffect, i32v3(mi.nx, mi.ny, mi.nz), mi.cutoutIndex, textureIndex, overlayTextureIndex, color, overlayColor, sunLight, lampLight, block.textures[0]);
            mi.cutoutIndex += 4;
            VoxelMesher::makeFloraFace(&(_cutoutVerts[0]), VoxelMesher::floraVertices[r], VoxelMesher::floraNormals, 24, block.waveEffect, i32v3(mi.nx, mi.ny, mi.nz), mi.cutoutIndex, textureIndex, overlayTextureIndex, color, overlayColor, sunLight, lampLight, block.textures[0]);
            mi.cutoutIndex += 4;
            break;
    }
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
    for (by = 1; by < m_dataWidth-1; by++) {
        for (bz = 1; bz < m_dataWidth-1; bz++){
            for (bx = 1; bx < m_dataWidth-1; bx++){
                // Get data for this voxel
                m_heightData = &chunkGridData->heightData[(bz - 1) * CHUNK_WIDTH + bx - 1];
                m_blockIndex = by * m_dataLayer + bz * m_dataWidth + bx;
                m_blockID = m_blockData[m_blockIndex];
                m_block = &m_blocks->operator[](m_blockID);

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

    int highestY = 0, lowestY = 256, highestX = 0, lowestX = 256, highestZ = 0, lowestZ = 256;
    int pyVboOff, pxVboOff, pzVboOff, nyVboOff, nxVboOff, nzVboOff;
    int index = 0;
    pyVboOff = 0;

    chunkMeshData->vertices.resize(mi.pyVboSize + mi.nxVboSize + mi.pxVboSize + mi.nzVboSize + mi.pzVboSize + mi.nyVboSize);
    for (int i = 0; i < mi.pyVboSize; i++){
        if (_finalTopVerts[i].position.y < lowestY) lowestY = _finalTopVerts[i].position.y;
        chunkMeshData->vertices[index++] = _finalTopVerts[i];
    }
    nxVboOff = index;
    for (int i = 0; i < mi.nxVboSize; i++){
        if (_finalLeftVerts[i].position.x > highestX) highestX = _finalLeftVerts[i].position.x;
        chunkMeshData->vertices[index++] = _finalLeftVerts[i];
    }
    pxVboOff = index;
    for (int i = 0; i < mi.pxVboSize; i++){
        if (_finalRightVerts[i].position.x < lowestX) lowestX = _finalRightVerts[i].position.x;
        chunkMeshData->vertices[index++] = _finalRightVerts[i];
    }
    nzVboOff = index;
    for (int i = 0; i < mi.nzVboSize; i++){
        if (_finalBackVerts[i].position.z > highestZ) highestZ = _finalBackVerts[i].position.z;
        chunkMeshData->vertices[index++] = _finalBackVerts[i];
    }
    pzVboOff = index;
    for (int i = 0; i < mi.pzVboSize; i++){
        if (_finalFrontVerts[i].position.z < lowestZ) lowestZ = _finalFrontVerts[i].position.z;
        chunkMeshData->vertices[index++] = _finalFrontVerts[i];
    }
    nyVboOff = index;
    for (int i = 0; i < mi.nyVboSize; i++){
        if (_finalBottomVerts[i].position.y > highestY) highestY = _finalBottomVerts[i].position.y;
        chunkMeshData->vertices[index++] = _finalBottomVerts[i];
    }

    if (mi.transparentIndex) {
        chunkMeshData->transVertices.swap(_transparentVerts);
        assert(chunkMeshData->transQuadIndices.size() != 0);
    }

    if (mi.cutoutIndex) {
        chunkMeshData->cutoutVertices.swap(_cutoutVerts);
    }

    highestY /= 7;
    lowestY /= 7;
    highestX /= 7;
    lowestX /= 7;
    highestZ /= 7;
    lowestZ /= 7;

    int indice = (index / 4) * 6;

    ChunkMeshRenderData& meshInfo = chunkMeshData->chunkMeshRenderData;

    //add all vertices to the vbo
    if (chunkMeshData->vertices.size() || chunkMeshData->transVertices.size() || chunkMeshData->cutoutVertices.size()) {
        meshInfo.indexSize = (chunkMeshData->vertices.size() * 6) / 4;

		//now let the sizes represent indice sizes
		meshInfo.pyVboOff = pyVboOff;
		meshInfo.pyVboSize = (mi.pyVboSize / 4) * 6;
		meshInfo.nyVboOff = nyVboOff;
		meshInfo.nyVboSize = (mi.nyVboSize / 4) * 6;
		meshInfo.pxVboOff = pxVboOff;
		meshInfo.pxVboSize = (mi.pxVboSize / 4) * 6;
		meshInfo.nxVboOff = nxVboOff;
		meshInfo.nxVboSize = (mi.nxVboSize / 4) * 6;
		meshInfo.pzVboOff = pzVboOff;
		meshInfo.pzVboSize = (mi.pzVboSize / 4) * 6;
		meshInfo.nzVboOff = nzVboOff;
		meshInfo.nzVboSize = (mi.nzVboSize / 4) * 6;

        meshInfo.transVboSize = (mi.transparentIndex / 4) * 6;
        meshInfo.cutoutVboSize = (mi.cutoutIndex / 4) * 6;
      
		meshInfo.highestY = highestY;
		meshInfo.lowestY = lowestY;
		meshInfo.highestX = highestX;
		meshInfo.lowestX = lowestX;
		meshInfo.highestZ = highestZ;
		meshInfo.lowestZ = lowestZ;
	}

	if (mi.liquidIndex){
        meshInfo.waterIndexSize = (mi.liquidIndex * 6) / 4;
        chunkMeshData->waterVertices.swap(_waterVboVerts);
	}

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
