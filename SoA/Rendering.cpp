#include "stdafx.h"
#include "Rendering.h"

#include <glm\gtc\matrix_transform.hpp>

#include "BlockData.h"
#include "VoxelMesher.h"
#include "Chunk.h"
#include "GLProgramManager.h"
#include "GameManager.h"
#include "ObjectLoader.h"
#include "OpenGLStructs.h"
#include "Options.h"
#include "Texture2d.h"
#include "Texture2d.h"
#include "WorldStructs.h"


// TODO: Remove This
using namespace glm;

int sunColor[64][3];

 GLushort starboxIndices[6][6];

GLfloat colorVertices[1024];
GLfloat cubeSpriteVerts[24];
GLfloat cubeSpriteUVs[24];
GLfloat cubeSpriteColorVertices[48] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
                                        0.8, 0.8, 0.8, 1.0, 0.8, 0.8, 0.8, 1.0, 0.8, 0.8, 0.8, 1.0, 0.8, 0.8, 0.8, 1.0,
                                        0.6, 0.6, 0.6, 1.0, 0.6, 0.6, 0.6, 1.0, 0.6, 0.6, 0.6, 1.0, 0.6, 0.6, 0.6, 1.0 };

static GLfloat physicsBlockVertices[72] = { -0.499f, 0.499f, 0.499f, -0.499f, -0.499f, 0.499f, 0.499f, -0.499f, 0.499f, 0.499f, 0.499f, 0.499f,  // v1-v2-v3-v0 (front)

0.499f, 0.499f, 0.499f, 0.499f, -0.499f, 0.499f, 0.499f, -0.499f, -0.499f, 0.499f, 0.499f, -0.499f,     // v0-v3-v4-v499 (right)

-0.499f, 0.499f, -0.499f, -0.499f, 0.499f, 0.499f, 0.499f, 0.499f, 0.499f, 0.499f, 0.499f, -0.499f,    // v6-v1-v0-v499 (top)

-0.499f, 0.499f, -0.499f, -0.499f, -0.499f, -0.499f, -0.499f, -0.499f, 0.499f, -0.499f, 0.499f, 0.499f,   // v6-v7-v2-v1 (left)

-0.499f, -0.499f, -0.499f, 0.499f, -0.499f, -0.499f, 0.499f, -0.499f, 0.499f, -0.499f, -0.499f, 0.499f,    // v7-v4-v3-v2 (bottom)

0.499f, 0.499f, -0.499f, 0.499f, -0.499f, -0.499f, -0.499f, -0.499f, -0.499f, -0.499f, 0.499f, -0.499f };     // v5-v4-v7-v6 (back)

GLfloat flatSpriteVertices[8] = {0, 60, 0, 0, 60, 0, 60, 60};

GLushort cubeSpriteDrawIndices[18] = {0,1,2,2,3,0,4,5,6,6,7,4,8,9,10,10,11,8};

BillboardVertex billVerts[BILLBOARD_VERTS_SIZE];
TreeVertex treeVerts[TREE_VERTS_SIZE];

WorldRenderer worldRenderer;

//Pre smooth mesh benchmark: 3567 ms

GLfloat image2dVertices[8];
//left, up, right, down
GLfloat image2dUVs[4][8] = { {1,1,0,1,0,0,1,0}, {0,1,0,0,1,0,1,1}, {0,0,1,0,1,1,0,1}, {1,0,1,1,0,1,0,0} };
GLushort image2dDrawIndices[6] = {0,1,2,2,3,0};

WorldRenderer::WorldRenderer()
{

}

WorldRenderer::~WorldRenderer()
{

}

void WorldRenderer::DrawLine(glm::vec3 a, glm::vec3 b)
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    vector <GLfloat> lineVertices(6);

    lineVertices[0] = a[0];
    lineVertices[1] = a[1];
    lineVertices[2] = a[2];
    lineVertices[3] = b[0];
    lineVertices[4] = b[1];
    lineVertices[5] = b[2];

    glVertexPointer(3, GL_FLOAT, 0, &(lineVertices[0]));

    glDrawArrays(GL_LINES, 0, 2);
}

GLfloat sunUVs[8];
GLfloat sunVerts[12];
GLushort sunIndices[6];

void DrawSun(float theta, glm::mat4 &MVP){

    double radius = 2800000.0;
    double size = 200000.0;
    float off = (float)atan(size/radius); //arctan(size/radius)  in radians
    float cosTheta = cos(theta - off);
    float sinTheta = sin(theta - off);
    float cosTheta2 = cos(theta + off);
    float sinTheta2 = sin(theta + off);

    // Bind shader
    vcore::GLProgram* program = GameManager::glProgramManager->getProgram("Texture");
    program->use();
    program->enableVertexAttribArrays();

    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

        // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sunTexture.ID);
    // Set our "myTextureSampler" sampler to user Texture Unit 0
    glUniform1i(program->getUniform("myTextureSampler"), 0);

    glUniformMatrix4fv(program->getUniform("MVP"), 1, GL_FALSE, &MVP[0][0]);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    sunUVs[0] = 0;
    sunUVs[1] = 0;
    sunUVs[2] = 0;
    sunUVs[3] = 1;
    sunUVs[4] = 1;
    sunUVs[5] = 1;
    sunUVs[6] = 1;
    sunUVs[7] = 0;

    sunIndices[0] = 0;
    sunIndices[1] = 1;
    sunIndices[2] = 2;
    sunIndices[3] = 2;
    sunIndices[4] = 3;
    sunIndices[5] = 0;

    sunVerts[0] = cosTheta2*radius;
    sunVerts[2] = sinTheta2*radius;
    sunVerts[1] = -size;

    sunVerts[3] = cosTheta*radius;
    sunVerts[5] = sinTheta*radius;
    sunVerts[4] = -size;

    sunVerts[6] = cosTheta*radius;
    sunVerts[8] = sinTheta*radius;
    sunVerts[7] = size;

    sunVerts[9] = cosTheta2*radius;
    sunVerts[11] = sinTheta2*radius;
    sunVerts[10] = size;

    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sunVerts), sunVerts, GL_STATIC_DRAW);

    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sunUVs), sunUVs, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // 2nd attribute buffer : UVs
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, sunIndices);

    program->disableVertexAttribArrays();
    program->unuse();
    
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);

    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
}

void DrawStars(float theta, glm::mat4 &MVP)
{
    glDisable(GL_CULL_FACE);

    // Bind shader
    vcore::GLProgram* program = GameManager::glProgramManager->getProgram("Texture");
    program->use();
    program->enableVertexAttribArrays();

        // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    // Set our "myTextureSampler" sampler to user Texture Unit 0
    glUniform1i(program->getUniform("myTextureSampler"), 0);

    glUniformMatrix4fv(program->getUniform("MVP"), 1, GL_FALSE, &MVP[0][0]);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(starboxVertices), starboxVertices, GL_STATIC_DRAW);

    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(starboxUVs), starboxUVs, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // 2nd attribute buffer : UVs
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    for (int i = 0; i < 6; i++){
        glBindTexture(GL_TEXTURE_2D, starboxTextures[i].ID);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, starboxIndices[i]); //offset
    }

    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);

    program->disableVertexAttribArrays();
    program->unuse();

    glEnable(GL_CULL_FACE);
}

void DrawWireBox(double x, double y, double z, double xw, double yh, double zw, float lineWidth, const glm::dvec3 &playerPos, glm::mat4 &VP, glm::vec4 color)
{
    GlobalModelMatrix[0][0] = xw;
    GlobalModelMatrix[1][1] = yh;
    GlobalModelMatrix[2][2] = zw;
    GlobalModelMatrix[3][0] = (float)((double)x - playerPos.x);
    GlobalModelMatrix[3][1] = (float)((double)y - playerPos.y);
    GlobalModelMatrix[3][2] = (float)((double)z - playerPos.z);

    glm::mat4 MVP = VP * GlobalModelMatrix;

    vcore::GLProgram* program = GameManager::glProgramManager->getProgram("BasicColor");

    program->use();
    program->enableVertexAttribArrays();

    glUniformMatrix4fv(program->getUniform("MVP"), 1, GL_FALSE, &MVP[0][0]);
    glUniform4f(program->getUniform("Color"), (GLfloat)color.r, (GLfloat)color.g, (GLfloat)color.b, (GLfloat)color.a);
    // Lazily construct vbo
    static ui32 vbo = 0;
    static ui32 ibo = 0;
    if (vbo == 0) {
        f32v3 lineVertices[8];
        GLushort elementBuffer[24] = { 0, 1, 0, 2, 1, 3, 2, 3, 4, 5, 4, 6, 5, 7, 6, 7, 0, 4, 1, 5, 2, 6, 3, 7 };
        float gmin = 0.00001;
        float gmax = 0.9999;
        lineVertices[0] = f32v3(gmin, gmin, gmin);
        //back right
        lineVertices[1] = f32v3(gmax, gmin, gmin); 
        //front left
        lineVertices[2] = f32v3(gmin, gmin, gmax);
        //front right
        lineVertices[3] = f32v3(gmax, gmin, gmax);
        // top 4
        //back left
        lineVertices[4] = f32v3(gmin, gmax, gmin);
        //back right
        lineVertices[5] = f32v3(gmax, gmax, gmin);
        //front left
        lineVertices[6] = f32v3(gmin, gmax, gmax);
        //front right
        lineVertices[7] = f32v3(gmax, gmax, gmax);

        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ibo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elementBuffer), elementBuffer, GL_STATIC_DRAW);
    } else {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    }

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glLineWidth(lineWidth);

    glDisable(GL_CULL_FACE);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, (void *)0);
    glEnable(GL_CULL_FACE);

    program->disableVertexAttribArrays();
    program->unuse();

    GlobalModelMatrix[0][0] = 1.0;
    GlobalModelMatrix[1][1] = 1.0;
    GlobalModelMatrix[2][2] = 1.0;
}

GLuint MakeBlockVbo(Block *block){

    static GLfloat ambientOcclusion[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    ColorRGB8 lampColor(100, 100, 100);
    ui8 sunlight = 80;
    vector <BlockVertex> vertices;
    vertices.resize(24);
    int btype = block->ID;

    ColorRGB8 sideColor, sideOverlayColor;
    ColorRGB8 topColor, topOverlayColor;
    ColorRGB8 botColor, botOverlayColor;

    Blocks[btype].GetBlockColor(sideColor, sideOverlayColor, 0, 128, 128, block->pxTexInfo);

    GLuint vboID;
    glGenBuffers(1, &vboID); // Create the buffer ID
    glBindBuffer(GL_ARRAY_BUFFER, vboID); // Bind the buffer (vertex array data)

    int index = 0;
    int sideOvTexOffset = 0; //for making inventory grass look better

    switch (block->meshType) {
        case MeshType::BLOCK:

            Blocks[btype].GetBlockColor(topColor, topOverlayColor, 0, 128, 128, block->pyTexInfo);
            Blocks[btype].GetBlockColor(botColor, botOverlayColor, 0, 128, 128, block->nyTexInfo);

            switch (block->pxTexInfo.overlay.method) {
                case ConnectedTextureMethods::GRASS:
                    sideOvTexOffset = 1;
                    break;
                default:
                    break;
            }

            //front
            VoxelMesher::makeCubeFace(vertices.data(), 1, CUBE_FACE_0_VERTEX_OFFSET, block->waveEffect, i32v3(0), index, block->base.pz, block->overlay.pz + sideOvTexOffset, sideColor, sideOverlayColor, ambientOcclusion, block->pzTexInfo);
            VoxelMesher::setFaceLight(vertices.data(), index, lampColor, sunlight);
            index += 4;
            //right
            VoxelMesher::makeCubeFace(vertices.data(), 1, CUBE_FACE_1_VERTEX_OFFSET, block->waveEffect, i32v3(0), index, block->base.px, block->overlay.px + sideOvTexOffset, sideColor, sideOverlayColor, ambientOcclusion, block->pxTexInfo);
            VoxelMesher::setFaceLight(vertices.data(), index, lampColor, sunlight);
            index += 4;
            //top
            VoxelMesher::makeCubeFace(vertices.data(), 1, CUBE_FACE_2_VERTEX_OFFSET, block->waveEffect, i32v3(0), index, block->base.py, block->overlay.py, topColor, topOverlayColor, ambientOcclusion, block->pyTexInfo);
            VoxelMesher::setFaceLight(vertices.data(), index, lampColor, sunlight);
            index += 4;
            //left
            VoxelMesher::makeCubeFace(vertices.data(), 1, CUBE_FACE_3_VERTEX_OFFSET, block->waveEffect, i32v3(0), index, block->base.nx, block->overlay.nx + sideOvTexOffset, sideColor, sideOverlayColor, ambientOcclusion, block->nxTexInfo);
            VoxelMesher::setFaceLight(vertices.data(), index, lampColor, sunlight);
            index += 4;
            //bottom
            VoxelMesher::makeCubeFace(vertices.data(), 1, CUBE_FACE_4_VERTEX_OFFSET, block->waveEffect, i32v3(0), index, block->base.ny, block->overlay.ny, botColor, botOverlayColor, ambientOcclusion, block->nyTexInfo);
            VoxelMesher::setFaceLight(vertices.data(), index, lampColor, sunlight);
            index += 4;
            //back
            VoxelMesher::makeCubeFace(vertices.data(), 1, CUBE_FACE_5_VERTEX_OFFSET, block->waveEffect, i32v3(0), index, block->base.nz, block->overlay.nz + sideOvTexOffset, sideColor, sideOverlayColor, ambientOcclusion, block->nzTexInfo);
            VoxelMesher::setFaceLight(vertices.data(), index, lampColor, sunlight);
            index += 4;

            glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(BlockVertex), NULL, GL_STATIC_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0, 24 * sizeof(BlockVertex), &(vertices[0]));
            break;
        case MeshType::FLORA:

            VoxelMesher::makeFloraFace(vertices.data(), VoxelMesher::floraVertices[0], VoxelMesher::floraNormals, 0, block->waveEffect, i32v3(0), index, block->base.pz, block->overlay.pz, sideColor, sideOverlayColor, sunlight, lampColor, block->pzTexInfo);
            index += 4;
            VoxelMesher::makeFloraFace(vertices.data(), VoxelMesher::floraVertices[0], VoxelMesher::floraNormals, 12, block->waveEffect, i32v3(0), index, block->base.px, block->overlay.px, sideColor, sideOverlayColor, sunlight, lampColor, block->pxTexInfo);
            index += 4;
            VoxelMesher::makeFloraFace(vertices.data(), VoxelMesher::floraVertices[0], VoxelMesher::floraNormals, 24, block->waveEffect, i32v3(0), index, block->base.py, block->overlay.py, sideColor, sideOverlayColor, sunlight, lampColor, block->pyTexInfo);
            index += 4;

            glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(BlockVertex), NULL, GL_STATIC_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0, 12 * sizeof(BlockVertex), &(vertices[0]));
            break;
        case MeshType::CROSSFLORA:
            VoxelMesher::makeFloraFace(vertices.data(), VoxelMesher::crossFloraVertices[0], VoxelMesher::floraNormals, 0, block->waveEffect, i32v3(0), index, block->base.pz, block->overlay.pz, sideColor, sideOverlayColor, sunlight, lampColor, block->pzTexInfo);
            index += 4;
            VoxelMesher::makeFloraFace(vertices.data(), VoxelMesher::crossFloraVertices[0], VoxelMesher::floraNormals, 12, block->waveEffect, i32v3(0), index, block->base.px, block->overlay.px, sideColor, sideOverlayColor, sunlight, lampColor, block->pxTexInfo);
            index += 4;

            glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(BlockVertex), NULL, GL_STATIC_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0, 8 * sizeof(BlockVertex), &(vertices[0]));
            break;
    }
   

    return vboID;
}

void Draw3DCube(Block *block, double x, double y, double z, glm::mat4 &VP, glm::mat4 &rotation){

    const float eyeDir[3] = { 0.0f, 0.0f, -1.0f };
    const float fadeDist = (GLfloat)10000.0f;
    const float blockAmbient = 0.000f;
    const glm::vec3 light = glm::vec3(glm::inverse(rotation) * glm::vec4(glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f)), 1.0));

    GlobalModelMatrix[3][0] = (x);
    GlobalModelMatrix[3][1] = (y);
    GlobalModelMatrix[3][2] = (z);

    glm::mat4 translation(1.0);
    translation[3][0] = -0.5;
    translation[3][1] = -0.5;
    translation[3][2] = -0.5;

    glm::mat4 M = GlobalModelMatrix * rotation * translation;
    glm::mat4 MVP = VP * M;

    vcore::GLProgram* program = nullptr;

    switch (block->meshType) {
        case MeshType::CROSSFLORA:
        case MeshType::FLORA:
            program = GameManager::glProgramManager->getProgram("Cutout");
            break;
        case MeshType::BLOCK:
        default:
            program = GameManager::glProgramManager->getProgram("Block");
            break;
    }

    program->use();
    program->enableVertexAttribArrays();

    // Set uniforms
    glUniform1f(program->getUniform("lightType"), 0); 
    glUniform3fv(program->getUniform("eyeNormalWorldspace"), 1, eyeDir);
    glUniform1f(program->getUniform("fogEnd"), (GLfloat)100000.0f);
    glUniform1f(program->getUniform("fogStart"), (GLfloat)10000.0f);
    glUniform3f(program->getUniform("fogColor"), 1.0f, 1.0f, 1.0f);
    glUniform3f(program->getUniform("lightPosition_worldspace"), light.x, light.y, light.z);
    glUniform1f(program->getUniform("specularExponent"), graphicsOptions.specularExponent);
    glUniform1f(program->getUniform("specularIntensity"), graphicsOptions.specularIntensity*0.3);
    glUniform1f(program->getUniform("dt"), (GLfloat)bdt);
    glUniform1f(program->getUniform("sunVal"), 1.0f);
    glUniform1f(program->getUniform("alphaMult"), 1.0f);
    glUniform3f(program->getUniform("ambientLight"), blockAmbient, blockAmbient, blockAmbient);
    glUniform3f(program->getUniform("lightColor"), (GLfloat)1.0f, (GLfloat)1.0f, (GLfloat)1.0f);
    glUniform1f(program->getUniform("fadeDistance"), fadeDist);
    glUniformMatrix4fv(program->getUniform("MVP"), 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(program->getUniform("M"), 1, GL_FALSE, &M[0][0]);

    // Bind the block textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, blockPack.textureInfo.ID);

    GLuint vboID = MakeBlockVbo(block);

    glDisable(GL_CULL_FACE);


    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Chunk::vboIndicesID);

    //position + texture type
    glVertexAttribPointer(0, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(BlockVertex), 0);
    //UV, animation, blendmode
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(BlockVertex), ((char *)NULL + (4)));
    //textureAtlas_textureIndex
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(BlockVertex), ((char *)NULL + (8)));
    //Texture dimensions
    glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(BlockVertex), ((char *)NULL + (12)));
    //color
    glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(BlockVertex), ((char *)NULL + (16)));
    //overlayColor
    glVertexAttribPointer(5, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(BlockVertex), ((char *)NULL + (20)));
    //lightcolor[3], sunlight,
    glVertexAttribPointer(6, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(BlockVertex), ((char *)NULL + (24)));
    //normal
    glVertexAttribPointer(7, 3, GL_BYTE, GL_TRUE, sizeof(BlockVertex), ((char *)NULL + (28)));
    
    switch (block->meshType) {
        case MeshType::BLOCK:
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            break;
        case MeshType::CROSSFLORA:
            glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
            break;
        case MeshType::FLORA:
            glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
            break;
    }

    program->disableVertexAttribArrays();
    program->unuse();

    glEnable(GL_CULL_FACE);

    glDeleteBuffers(1, &vboID);


}