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

#define NO_QUAD_INDEX 0xFFFF

#define QUAD_SIZE 7

#define USE_AO

// Base texture index
#define B_INDEX 0
// Overlay texture index
#define O_INDEX 1

const int X_NEG = (int)vvox::Cardinal::X_NEG;
const int X_POS = (int)vvox::Cardinal::X_POS;
const int Y_NEG = (int)vvox::Cardinal::Y_NEG;
const int Y_POS = (int)vvox::Cardinal::Y_POS;
const int Z_NEG = (int)vvox::Cardinal::Z_NEG;
const int Z_POS = (int)vvox::Cardinal::Z_POS;

#define UV_0 128
#define UV_1 129

// Meshing constants
//0 = x, 1 = y, 2 = z
const int FACE_AXIS[6][2] = { { 2, 1 }, { 2, 1 }, { 0, 2 }, { 0, 2 }, { 0, 1 }, { 0, 1 } };

const int FACE_AXIS_SIGN[6][2] = { { 1, 1 }, { -1, 1 }, { 1, -1 }, { -1, -1 }, { -1, 1 }, { 1, 1 } };

void ChunkMesher::init(const BlockPack* blocks) {
    this->blocks = blocks;

    // Set up the texture params
    m_textureMethodParams[X_NEG][B_INDEX].init(this, PADDED_CHUNK_WIDTH, PADDED_CHUNK_LAYER, -1, offsetof(BlockTextureFaces, BlockTextureFaces::nx) / sizeof(ui32));
    m_textureMethodParams[X_NEG][O_INDEX].init(this, PADDED_CHUNK_WIDTH, PADDED_CHUNK_LAYER, -1, offsetof(BlockTextureFaces, BlockTextureFaces::nx) / sizeof(ui32) + NUM_FACES);

    m_textureMethodParams[X_POS][B_INDEX].init(this, -PADDED_CHUNK_WIDTH, PADDED_CHUNK_LAYER, 1, offsetof(BlockTextureFaces, BlockTextureFaces::px) / sizeof(ui32));
    m_textureMethodParams[X_POS][O_INDEX].init(this, -PADDED_CHUNK_WIDTH, PADDED_CHUNK_LAYER, 1, offsetof(BlockTextureFaces, BlockTextureFaces::px) / sizeof(ui32) + NUM_FACES);

    m_textureMethodParams[Y_NEG][B_INDEX].init(this, -1, -PADDED_CHUNK_WIDTH, -PADDED_CHUNK_LAYER, offsetof(BlockTextureFaces, BlockTextureFaces::ny) / sizeof(ui32));
    m_textureMethodParams[Y_NEG][O_INDEX].init(this, -1, -PADDED_CHUNK_WIDTH, -PADDED_CHUNK_LAYER, offsetof(BlockTextureFaces, BlockTextureFaces::ny) / sizeof(ui32) + NUM_FACES);

    m_textureMethodParams[Y_POS][B_INDEX].init(this, 1, -PADDED_CHUNK_WIDTH, PADDED_CHUNK_LAYER, offsetof(BlockTextureFaces, BlockTextureFaces::py) / sizeof(ui32));
    m_textureMethodParams[Y_POS][O_INDEX].init(this, 1, -PADDED_CHUNK_WIDTH, PADDED_CHUNK_LAYER, offsetof(BlockTextureFaces, BlockTextureFaces::py) / sizeof(ui32) + NUM_FACES);

    m_textureMethodParams[Z_NEG][B_INDEX].init(this, -1, PADDED_CHUNK_LAYER, -PADDED_CHUNK_WIDTH, offsetof(BlockTextureFaces, BlockTextureFaces::nz) / sizeof(ui32));
    m_textureMethodParams[Z_NEG][O_INDEX].init(this, -1, PADDED_CHUNK_LAYER, -PADDED_CHUNK_WIDTH, offsetof(BlockTextureFaces, BlockTextureFaces::nz) / sizeof(ui32) + NUM_FACES);

    m_textureMethodParams[Z_POS][B_INDEX].init(this, 1, PADDED_CHUNK_LAYER, PADDED_CHUNK_WIDTH, offsetof(BlockTextureFaces, BlockTextureFaces::pz) / sizeof(ui32));
    m_textureMethodParams[Z_POS][O_INDEX].init(this, 1, PADDED_CHUNK_LAYER, PADDED_CHUNK_WIDTH, offsetof(BlockTextureFaces, BlockTextureFaces::pz) / sizeof(ui32) + NUM_FACES);
}

bool ChunkMesher::createChunkMesh(RenderTask *renderTask) {
    m_numQuads = 0;
    m_highestY = 0;
    m_lowestY = 256;
    m_highestX = 0;
    m_lowestX = 256;
    m_highestZ = 0;
    m_lowestZ = 256;

    // Clear quad indices
    memset(m_quadIndices, 0xFF, sizeof(m_quadIndices));

    for (int i = 0; i < 6; i++) {
        m_quads[i].clear();
    }

    // CONST?
    Chunk* chunk = renderTask->chunk;

    // Here?
    _waterVboVerts.clear();
    _transparentVerts.clear();
    _cutoutVerts.clear();

    //create a new chunk mesh data container
    if (chunkMeshData != NULL) {
        pError("Tried to create mesh with in use chunkMeshData!");
        return 0;
    }

    //Stores the data for a chunk mesh
    // TODO(Ben): new is bad mkay
    chunkMeshData = new ChunkMeshData(renderTask);

    // Init the mesh info
    // Redundant
    //mi.init(m_blocks, m_dataWidth, m_dataLayer);

    // Loop through blocks
    for (by = 0; by < PADDED_CHUNK_WIDTH - 2; by++) {
        for (bz = 0; bz < PADDED_CHUNK_WIDTH - 2; bz++) {
            for (bx = 0; bx < PADDED_CHUNK_WIDTH - 2; bx++) {
                // Get data for this voxel
                // TODO(Ben): Could optimize out -1
                blockIndex = (by + 1) * PADDED_CHUNK_LAYER + (bz + 1) * PADDED_CHUNK_WIDTH + (bx + 1);
                blockID = blockData[blockIndex];
                if (blockID == 0) continue; // Skip air blocks
                heightData = &chunkGridData->heightData[(bz - 1) * CHUNK_WIDTH + bx - 1];
                block = &blocks->operator[](blockID);
                // TODO(Ben) Don't think bx needs to be member
                voxelPosOffset = ui8v3(bx * QUAD_SIZE, by * QUAD_SIZE, bz * QUAD_SIZE);

                switch (block->meshType) {
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
    std::vector<VoxelQuad>& finalQuads = chunkMeshData->opaqueQuads;

    finalQuads.resize(m_numQuads);
    // Copy the data
    // TODO(Ben): Could construct in place and not need ANY copying with 6 iterations?
    i32 index = 0;
    i32 sizes[6];
    for (int i = 0; i < 6; i++) {
        std::vector<VoxelQuad>& quads = m_quads[i];
        int tmp = index;
        for (size_t j = 0; j < quads.size(); j++) {
            VoxelQuad& q = quads[j];
            if (q.v0.mesherFlags & MESH_FLAG_ACTIVE) {
                finalQuads[index++] = q;
            }
        }
        sizes[i] = index - tmp;
    }

    m_highestY /= QUAD_SIZE;
    m_lowestY /= QUAD_SIZE;
    m_highestX /= QUAD_SIZE;
    m_lowestX /= QUAD_SIZE;
    m_highestZ /= QUAD_SIZE;
    m_lowestZ /= QUAD_SIZE;

#define INDICES_PER_QUAD 6

    if (finalQuads.size()) {
        renderData.nxVboOff = 0;
        renderData.nxVboSize = sizes[0] * INDICES_PER_QUAD;
        renderData.pxVboOff = renderData.nxVboSize;
        renderData.pxVboSize = sizes[1] * INDICES_PER_QUAD;
        renderData.nyVboOff = renderData.pxVboOff + renderData.pxVboSize;
        renderData.nyVboSize = sizes[2] * INDICES_PER_QUAD;
        renderData.pyVboOff = renderData.nyVboOff + renderData.nyVboSize;
        renderData.pyVboSize = sizes[3] * INDICES_PER_QUAD;
        renderData.nzVboOff = renderData.pyVboOff + renderData.pyVboSize;
        renderData.nzVboSize = sizes[4] * INDICES_PER_QUAD;
        renderData.pzVboOff = renderData.nzVboOff + renderData.nzVboSize;
        renderData.pzVboSize = sizes[5] * INDICES_PER_QUAD;
        renderData.indexSize = finalQuads.size() * INDICES_PER_QUAD;

        // Redundant
        renderData.highestX = m_highestX;
        renderData.lowestX = m_lowestX;
        renderData.highestY = m_highestY;
        renderData.lowestY = m_lowestY;
        renderData.highestZ = m_highestZ;
        renderData.lowestZ = m_lowestZ;
    }

    return 0;
}

bool ChunkMesher::createOnlyWaterMesh(RenderTask *renderTask) {
    /*if (chunkMeshData != NULL) {
        pError("Tried to create mesh with in use chunkMeshData!");
        return 0;
        }
        chunkMeshData = new ChunkMeshData(renderTask);

        _waterVboVerts.clear();

        mi.task = renderTask;

        for (int i = 0; i < wSize; i++) {
        mi.wc = m_wvec[i];
        mi.btype = GETBLOCKID(blockData[mi.wc]);
        mi.x = (mi.wc % PADDED_CHUNK_WIDTH) - 1;
        mi.y = (mi.wc / PADDED_CHUNK_LAYER) - 1;
        mi.z = ((mi.wc % PADDED_CHUNK_LAYER) / PADDED_CHUNK_WIDTH) - 1;

        addLiquid(mi);
        }


        if (mi.liquidIndex) {
        chunkMeshData->chunkMeshRenderData.waterIndexSize = (mi.liquidIndex * 6) / 4;
        chunkMeshData->waterVertices.swap(_waterVboVerts);
        }*/

    return false;
}

void ChunkMesher::freeBuffers() {
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

void ChunkMesher::addBlock()
{
    // Ambient occlusion buffer for vertices
    f32 ao[4];

    // Check the faces
    // Left
    if (shouldRenderFace(-1)) {
        computeAmbientOcclusion(-1, -PADDED_CHUNK_LAYER, PADDED_CHUNK_WIDTH, ao);
        addQuad(X_NEG, (int)vvox::Axis::Z, (int)vvox::Axis::Y, -PADDED_CHUNK_WIDTH, -PADDED_CHUNK_LAYER, 2, ao);
    }
    // Right
    if (shouldRenderFace(1)) {
        computeAmbientOcclusion(1, -PADDED_CHUNK_LAYER, -PADDED_CHUNK_WIDTH, ao);
        addQuad(X_POS, (int)vvox::Axis::Z, (int)vvox::Axis::Y, -PADDED_CHUNK_WIDTH, -PADDED_CHUNK_LAYER, 0, ao);
    }
    // Bottom
    if (shouldRenderFace(-PADDED_CHUNK_LAYER)) { 
        computeAmbientOcclusion(-PADDED_CHUNK_LAYER, PADDED_CHUNK_WIDTH, 1, ao);
        addQuad(Y_NEG, (int)vvox::Axis::X, (int)vvox::Axis::Z, -1, -PADDED_CHUNK_WIDTH, 2, ao);
    }
    // Top
    if (shouldRenderFace(PADDED_CHUNK_LAYER)) {
        computeAmbientOcclusion(PADDED_CHUNK_LAYER, -PADDED_CHUNK_WIDTH, -1, ao);
        addQuad(Y_POS, (int)vvox::Axis::X, (int)vvox::Axis::Z, -1, -PADDED_CHUNK_WIDTH, 0, ao);
    }
    // Back
    if (shouldRenderFace(-PADDED_CHUNK_WIDTH)) {
        computeAmbientOcclusion(-PADDED_CHUNK_WIDTH, -PADDED_CHUNK_LAYER, -1, ao);
        addQuad(Z_NEG, (int)vvox::Axis::X, (int)vvox::Axis::Y, -1, -PADDED_CHUNK_LAYER, 0, ao);
    }
    // Front
    if (shouldRenderFace(PADDED_CHUNK_WIDTH)) {
        computeAmbientOcclusion(PADDED_CHUNK_WIDTH, -PADDED_CHUNK_LAYER, 1, ao);
        addQuad(Z_POS, (int)vvox::Axis::X, (int)vvox::Axis::Y, -1, -PADDED_CHUNK_LAYER, 2, ao);
    }
}

void ChunkMesher::computeAmbientOcclusion(int upOffset, int frontOffset, int rightOffset, f32 ambientOcclusion[]) {

    // Ambient occlusion factor
#define OCCLUSION_FACTOR 0.2f;
    // Helper macro
    // TODO(Ben): This isn't exactly right since self will occlude. Use a function
#define CALCULATE_VERTEX(v, s1, s2) \
    nearOccluders = getOcclusion(blocks->operator[](blockData[blockIndex])) + \
    getOcclusion(blocks->operator[](blockData[blockIndex s1 frontOffset])) + \
    getOcclusion(blocks->operator[](blockData[blockIndex s2 rightOffset])) + \
    getOcclusion(blocks->operator[](blockData[blockIndex s1 frontOffset s2 rightOffset])); \
    ambientOcclusion[v] = 1.0f - nearOccluders * OCCLUSION_FACTOR; 
   
    // Move the block index upwards
    int blockIndex = this->blockIndex + upOffset;
    int nearOccluders; ///< For ambient occlusion

    // TODO(Ben): I know for a fact the inputs are wrong for some faces

    // Vertex 0
    CALCULATE_VERTEX(0, -, -)

    // Vertex 1
    CALCULATE_VERTEX(1, +, -)

    // Vertex 2
    CALCULATE_VERTEX(2, +, +)

    // Vertex 3
    CALCULATE_VERTEX(3, -, +)
}

void ChunkMesher::addQuad(int face, int rightAxis, int frontAxis, int leftOffset, int backOffset, int rightStretchIndex, f32 ambientOcclusion[]) {
    // Get texture TODO(Ben): Null check?
    const BlockTexture* texture = block->textures[face];
    // Get color
    // TODO(Ben): Flags?
    color3 blockColor[2];
    block->getBlockColor(blockColor[0], blockColor[1],
                           0,
                           heightData->temperature,
                           heightData->rainfall,
                           texture);

    std::vector<VoxelQuad>& quads = m_quads[face];

    // Get texturing parameters
    ui8 blendMode = getBlendMode(texture->blendMode);
    // TODO(Ben): Get an offset instead?
    BlockTextureIndex baseTextureIndex = texture->base.getBlockTextureIndex(m_textureMethodParams[face][B_INDEX], blockColor[0]);
    BlockTextureIndex overlayTextureIndex = texture->base.getBlockTextureIndex(m_textureMethodParams[face][O_INDEX], blockColor[0]);
    ui8 baseTextureAtlas = (ui8)(baseTextureIndex / ATLAS_SIZE);
    ui8 overlayTextureAtlas = (ui8)(overlayTextureIndex / ATLAS_SIZE);
    baseTextureIndex %= ATLAS_SIZE;
    overlayTextureIndex %= ATLAS_SIZE;
    i32v3 pos(bx, by, bz);
    int uOffset = (ui8)(pos[FACE_AXIS[face][0]] * FACE_AXIS_SIGN[face][0]);
    int vOffset = (ui8)(pos[FACE_AXIS[face][1]] * FACE_AXIS_SIGN[face][1]);

    // Construct the quad
    i16 quadIndex = quads.size();
    quads.emplace_back();
    m_numQuads++;
    VoxelQuad* quad = &quads.back();
    quad->v0.mesherFlags = MESH_FLAG_ACTIVE;
    for (int i = 0; i < 4; i++) {
        BlockVertex& v = quad->verts[i];
        v.position = VoxelMesher::VOXEL_POSITIONS[face][i] + voxelPosOffset;
#ifdef USE_AO
        f32& ao = ambientOcclusion[i];
        v.color.r = (ui8)(blockColor[B_INDEX].r * ao);
        v.color.g = (ui8)(blockColor[B_INDEX].g * ao);
        v.color.b = (ui8)(blockColor[B_INDEX].b * ao);
        v.overlayColor.r = (ui8)(blockColor[O_INDEX].r * ao);
        v.overlayColor.g = (ui8)(blockColor[O_INDEX].g * ao);
        v.overlayColor.b = (ui8)(blockColor[O_INDEX].b * ao);
#else
        v.color = blockColor[0];
        v.overlayColor = blockColor[1];
#endif
        v.textureIndex = baseTextureIndex;
        v.textureAtlas = baseTextureAtlas;
        v.overlayTextureIndex = overlayTextureIndex;
        v.overlayTextureAtlas = overlayTextureAtlas;
        v.textureWidth = (ui8)texture->base.size.x;
        v.textureHeight = (ui8)texture->base.size.y;
        v.overlayTextureWidth = (ui8)texture->overlay.size.x;
        v.overlayTextureHeight = (ui8)texture->overlay.size.y;
        v.blendMode = blendMode;
        v.face = (ui8)face;
    }
    // Set texture coordinates
    quad->verts[0].tex.x = (ui8)(UV_0 + uOffset);
    quad->verts[0].tex.y = (ui8)(UV_1 + vOffset);
    quad->verts[1].tex.x = (ui8)(UV_0 + uOffset);
    quad->verts[1].tex.y = (ui8)(UV_0 + vOffset);
    quad->verts[2].tex.x = (ui8)(UV_1 + uOffset);
    quad->verts[2].tex.y = (ui8)(UV_0 + vOffset);
    quad->verts[3].tex.x = (ui8)(UV_1 + uOffset);
    quad->verts[3].tex.y = (ui8)(UV_1 + vOffset);

    // Check against lowest and highest for culling in render
    // TODO(Ben): Think about this more
    if (quad->v0.position.x < m_lowestX) m_lowestX = quad->v0.position.x;
    if (quad->v0.position.x > m_highestX) m_highestX = quad->v0.position.x;
    if (quad->v0.position.y < m_lowestY) m_lowestY = quad->v0.position.y;
    if (quad->v0.position.y > m_highestY) m_highestY = quad->v0.position.y;
    if (quad->v0.position.z < m_lowestZ) m_lowestZ = quad->v0.position.z;
    if (quad->v0.position.z > m_highestZ) m_highestZ = quad->v0.position.z;

    { // Look-Behind Greedy Merging(tm)
        if (quad->v0 == quad->v3 && quad->v1 == quad->v2) {
            quad->v0.mesherFlags |= MESH_FLAG_MERGE_RIGHT;
            ui16 leftIndex = m_quadIndices[blockIndex + leftOffset][face];
            // Check left merge
            if (leftIndex != NO_QUAD_INDEX) {
                VoxelQuad& lQuad = quads[leftIndex];
                if (((lQuad.v0.mesherFlags & MESH_FLAG_MERGE_RIGHT) != 0) &&
                    lQuad.v0.position.z == quad->v0.position.z &&
                    lQuad.v1.position.z == quad->v1.position.z &&
                    lQuad.v0 == quad->v0 && lQuad.v1 == quad->v1) {
                    // Stretch the previous quad
                    lQuad.verts[rightStretchIndex].position[rightAxis] += QUAD_SIZE;
                    lQuad.verts[rightStretchIndex].tex.x++;
                    lQuad.verts[rightStretchIndex + 1].position[rightAxis] += QUAD_SIZE;
                    lQuad.verts[rightStretchIndex + 1].tex.x++;
                    // Remove the current quad
                    quads.pop_back();
                    m_numQuads--;
                    quadIndex = leftIndex;
                    quad = &lQuad;
                }
            }
        }
        // Check back merge
        if (quad->v0 == quad->v1 && quad->v2 == quad->v3) {
            quad->v0.mesherFlags |= MESH_FLAG_MERGE_FRONT;
            int backIndex = m_quadIndices[blockIndex + backOffset][face];
            if (backIndex != NO_QUAD_INDEX) {
                VoxelQuad* bQuad = &quads[backIndex];
                while (!(bQuad->v0.mesherFlags & MESH_FLAG_ACTIVE)) {
                    backIndex = bQuad->replaceQuad;
                    bQuad = &quads[backIndex];
                }
                if (((bQuad->v0.mesherFlags & MESH_FLAG_MERGE_FRONT) != 0) &&
                    bQuad->v0.position[rightAxis] == quad->v0.position[rightAxis] &&
                    bQuad->v2.position[rightAxis] == quad->v2.position[rightAxis] &&
                    bQuad->v0 == quad->v0 && bQuad->v1 == quad->v1) {
                    bQuad->v0.position[frontAxis] += QUAD_SIZE;
                    bQuad->v0.tex.y++;
                    bQuad->v3.position[frontAxis] += QUAD_SIZE;
                    bQuad->v3.tex.y++;
                    quadIndex = backIndex;
                    // Mark as not in use
                    quad->v0.mesherFlags = 0;
                    quad->replaceQuad = backIndex;
                    m_numQuads--;
                }
            }
        }
    }
    
    // Mark quadIndices so we can merge this quad later
    m_quadIndices[blockIndex][face] = quadIndex;
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
void ChunkMesher::addLiquid() {
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
    int val = GETBLOCKID(blockData[blockIndex]); // Get block ID
    val = val - block.liquidStartID;
    if (val < 0) return 0;
    if (val > block.liquidLevels) return 0;
    return val;
}

bool ChunkMesher::shouldRenderFace(int offset) {
    const Block& neighbor = blocks->operator[](blockData[blockIndex + offset]);
    if (neighbor.occlude == BlockOcclusion::ALL) return false;
    if ((neighbor.occlude == BlockOcclusion::SELF) && (blockID == neighbor.ID)) return false;
    return true;
}

int ChunkMesher::getOcclusion(const Block& block) {
    if (block.occlude == BlockOcclusion::ALL) return 1;
    if ((block.occlude == BlockOcclusion::SELF) && (blockID == block.ID)) return 1;
    return 0;
}

ui8 ChunkMesher::getBlendMode(const BlendType& blendType) {
    // Shader interprets this with bitwise ops
    ubyte blendMode = 0x14; //0x14 = 00 01 01 00
    switch (blendType) {
        case BlendType::ALPHA:
            blendMode |= 1; //Sets blendMode to 00 01 01 01
            break;
        case BlendType::ADD:
            blendMode += 4; //Sets blendMode to 00 01 10 00
            break;
        case BlendType::SUBTRACT:
            blendMode -= 4; //Sets blendMode to 00 01 00 00
            break;
        case BlendType::MULTIPLY:
            blendMode -= 16; //Sets blendMode to 00 00 01 00
            break;
    }
    return blendMode;
}