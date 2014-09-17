#include "stdafx.h"
#include "Rendering.h"

#include <glm\gtc\matrix_transform.hpp>

#include "BlockData.h"
#include "Chunk.h"
#include "ObjectLoader.h"
#include "OpenGLStructs.h"
#include "Options.h"
#include "Texture2d.h"
#include "Texture2d.h"
#include "WorldStructs.h"
#include "shader.h"

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

void DrawCubeSpriteImage2D(GLfloat *vertices, int sizeOfvertices, GLfloat *uvs, int sizeOfuvs, GLushort *indices,int sizeOfindices, GLuint textureID, glm::vec4 color)
{
    glBindBuffer(GL_ARRAY_BUFFER, texture2Dshader.Text2DVertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeOfvertices, vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, texture2Dshader.Text2DUVBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeOfuvs, uvs, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, texture2Dshader.Text2DElementBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeOfindices, indices, GL_STATIC_DRAW);

    int j = 0;
    for (int i = 0; i < sizeOfvertices; i+=2, j+=4){
        colorVertices[j] = color[0]*cubeSpriteColorVertices[j];
        colorVertices[j+1] = color[1]*cubeSpriteColorVertices[j+1];
        colorVertices[j+2] = color[2]*cubeSpriteColorVertices[j+2];
        colorVertices[j+3] = color[3]*cubeSpriteColorVertices[j+3];
    }

    glBindBuffer(GL_ARRAY_BUFFER, texture2Dshader.Text2DColorBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeOfvertices*2, colorVertices, GL_STATIC_DRAW);

    // Bind shader
    texture2Dshader.Bind();

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    // Set our "myTextureSampler" sampler to user Texture Unit 0
    glUniform1i(texture2Dshader.Text2DUniformID, 0);
    glUniform1f(texture2Dshader.Text2DUseRoundMaskID, 0.0f);

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, texture2Dshader.Text2DVertexBufferID);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );

    // 2nd attribute buffer : UVs
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, texture2Dshader.Text2DUVBufferID);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );

    // 3rd attribute buffer : Colors
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, texture2Dshader.Text2DColorBufferID);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void*)0 );

    // Draw call
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, texture2Dshader.Text2DElementBufferID);
    glDrawElements(GL_TRIANGLES, sizeOfindices/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);

    texture2Dshader.UnBind();
}

//void DrawImage2D(GLfloat *vertices, int sizeOfvertices, GLfloat *uvs, int sizeOfuvs, GLushort *indices,int sizeOfindices, GLuint textureID, glm::vec4 color, bool roundMask, float xdim, float ydim)
//{
//    glBindBuffer(GL_ARRAY_BUFFER, texture2Dshader.Text2DVertexBufferID);
//    glBufferData(GL_ARRAY_BUFFER, sizeOfvertices, vertices, GL_STATIC_DRAW);
//    glBindBuffer(GL_ARRAY_BUFFER, texture2Dshader.Text2DUVBufferID);
//    glBufferData(GL_ARRAY_BUFFER, sizeOfuvs, uvs, GL_STATIC_DRAW);
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, texture2Dshader.Text2DElementBufferID);
//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeOfindices, indices, GL_STATIC_DRAW);
//
//    int j = 0;
//    for (int i = 0; i < sizeOfvertices; i+=2, j+=4){
//        colorVertices[j] = color[0];
//        colorVertices[j+1] = color[1];
//        colorVertices[j+2] = color[2];
//        colorVertices[j+3] = color[3];
//    }
//
//    glBindBuffer(GL_ARRAY_BUFFER, texture2Dshader.Text2DColorBufferID);
//    glBufferData(GL_ARRAY_BUFFER, sizeOfvertices*2, colorVertices, GL_STATIC_DRAW);
//
//    // Bind shader
//    texture2Dshader.Bind(xdim, ydim);
//    if (roundMask){
//        glUniform2f(texture2Dshader.Text2DStartUVID, uvs[2], uvs[3]);
//        glUniform1i(texture2Dshader.Text2DRoundMaskID, 6);
//        glUniform1f(texture2Dshader.Text2DUseRoundMaskID, 1.0f);
//    }else{
//        glUniform1f(texture2Dshader.Text2DUseRoundMaskID, 0.0f);
//    }
//
//    // Bind texture
//    glActiveTexture(GL_TEXTURE0);
//    glBindTexture(GL_TEXTURE_2D, textureID);
//    // Set our "myTextureSampler" sampler to user Texture Unit 0
//    glUniform1i(texture2Dshader.Text2DUniformID, 0);
//
//    // 1rst attribute buffer : vertices
//    glEnableVertexAttribArray(0);
//    glBindBuffer(GL_ARRAY_BUFFER, texture2Dshader.Text2DVertexBufferID);
//    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );
//
//    // 2nd attribute buffer : UVs
//    glEnableVertexAttribArray(1);
//    glBindBuffer(GL_ARRAY_BUFFER, texture2Dshader.Text2DUVBufferID);
//    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );
//
//    // 3rd attribute buffer : Colors
//    glEnableVertexAttribArray(2);
//    glBindBuffer(GL_ARRAY_BUFFER, texture2Dshader.Text2DColorBufferID);
//    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void*)0 );
//
//    // Draw call
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, texture2Dshader.Text2DElementBufferID);
//    glDrawElements(GL_TRIANGLES, sizeOfindices/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
//
//    texture2Dshader.UnBind();
//}

GLfloat image2dVertices[8];
//left, up, right, down
GLfloat image2dUVs[4][8] = { {1,1,0,1,0,0,1,0}, {0,1,0,0,1,0,1,1}, {0,0,1,0,1,1,0,1}, {1,0,1,1,0,1,0,0} };
GLushort image2dDrawIndices[6] = {0,1,2,2,3,0};

//Oreintation 0 = left, 1 = up, 2 = right, 3 = down
void DrawImage2D(float x, float y, float width, float height, GLuint textureID, float xdim, float ydim, glm::vec4 color, int oreintation)
{
    image2dVertices[0] = (GLfloat)x;
    image2dVertices[1] = (GLfloat)y + height;
    image2dVertices[2] = (GLfloat)x;
    image2dVertices[3] = (GLfloat)y;
    image2dVertices[4] = (GLfloat)x + width;
    image2dVertices[5] = (GLfloat)y;
    image2dVertices[6] = (GLfloat)x + width;
    image2dVertices[7] = (GLfloat)y + height;

    glBindBuffer(GL_ARRAY_BUFFER, texture2Dshader.Text2DVertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(image2dVertices), image2dVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, texture2Dshader.Text2DUVBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(image2dUVs[oreintation]), image2dUVs[oreintation], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, texture2Dshader.Text2DElementBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(image2dDrawIndices), image2dDrawIndices, GL_STATIC_DRAW);

    int j = 0;
    for (int i = 0; i < sizeof(image2dVertices); i+=2, j+=4){
        colorVertices[j] = color[0];
        colorVertices[j+1] = color[1];
        colorVertices[j+2] = color[2];
        colorVertices[j+3] = color[3];
    }

    glBindBuffer(GL_ARRAY_BUFFER, texture2Dshader.Text2DColorBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(image2dVertices)*2, colorVertices, GL_STATIC_DRAW);

    // Bind shader
    texture2Dshader.Bind(xdim, ydim);
    glUniform1f(texture2Dshader.Text2DUseRoundMaskID, 0.0f);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    // Set our "myTextureSampler" sampler to user Texture Unit 0
    glUniform1i(texture2Dshader.Text2DUniformID, 0);

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, texture2Dshader.Text2DVertexBufferID);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );

    // 2nd attribute buffer : UVs
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, texture2Dshader.Text2DUVBufferID);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );

    // 3rd attribute buffer : Colors
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, texture2Dshader.Text2DColorBufferID);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void*)0 );

    // Draw call
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, texture2Dshader.Text2DElementBufferID);
    glDrawElements(GL_TRIANGLES, sizeof(image2dDrawIndices)/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);

    texture2Dshader.UnBind();
}

GLuint gridvboID, gridelID;
WorldRenderer::WorldRenderer()
{
    gridvboID = 0;
    gridelID = 0;
}

WorldRenderer::~WorldRenderer()
{
    if (gridvboID){
        glDeleteBuffers(1, &gridvboID);
        glDeleteBuffers(1, &gridelID);
    }
}

void WorldRenderer::Initialize()
{
    float lineVertices[8][3];
    GLushort elementBuffer[24] = { 0, 1, 0, 2, 1, 3, 2, 3, 4, 5, 4, 6, 5, 7, 6, 7, 0, 4, 1, 5, 2, 6, 3, 7 };

    float gmin = 0.00001;
    float gmax = 0.9999;
    lineVertices[0][0] = gmin;
    lineVertices[0][1] = gmin;
    lineVertices[0][2] = gmin;
    //back right
    lineVertices[1][0] = gmax;
    lineVertices[1][1] = gmin;
    lineVertices[1][2] = gmin;
    //front left
    lineVertices[2][0] = gmin;
    lineVertices[2][1] = gmin;
    lineVertices[2][2] = gmax;
    //front right
    lineVertices[3][0] = gmax;
    lineVertices[3][1] = gmin;
    lineVertices[3][2] = gmax;
    //       top 4
    //back left
    lineVertices[4][0] = gmin;
    lineVertices[4][1] = gmax;
    lineVertices[4][2] = gmin;
    //back right
    lineVertices[5][0] = gmax;
    lineVertices[5][1] = gmax;
    lineVertices[5][2] = gmin;
    //front left
    lineVertices[6][0] = gmin;
    lineVertices[6][1] = gmax;
    lineVertices[6][2] = gmax;
    //front right
    lineVertices[7][0] = gmax;
    lineVertices[7][1] = gmax;
    lineVertices[7][2] = gmax;

    glGenBuffers(1, &gridvboID);
    glGenBuffers(1, &gridelID);

    glBindBuffer(GL_ARRAY_BUFFER, gridvboID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gridelID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elementBuffer), elementBuffer, GL_STATIC_DRAW);
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

void DrawFullScreenQuad(glm::vec4 color)
{
    Texture2D wholeTexture;
    wholeTexture.Initialize(BlankTextureID.ID, 0, 0, screenWidth2d, screenHeight2d, color);
    wholeTexture.Draw();
}

void DrawRoundSprite(GLfloat x, GLfloat y, int blockType, float scale)
{
    /*int frontTex;
    GLuint texunit = Blocks[blockType].topTexUnit;

    Texture2D texture;

    for (int i = 0; i < 8; i+=2){
        cubeSpriteVerts[i] = flatSpriteVertices[i]*scale+x;
        cubeSpriteVerts[i+1] = flatSpriteVertices[i+1]*scale+y;
    }
    
    frontTex = Blocks[blockType].pxTex;

    float diff = ABS(blockTextureUVs[frontTex].u1 - blockTextureUVs[frontTex].u3);

    texture.Initialize(blockPacks[texunit].textureInfo.ID, x, y, scale, scale, glm::vec4((float)Blocks[blockType].tr/255.0f,(float)Blocks[blockType].tg/255.0f,(float)Blocks[blockType].tb/255.0f, 1.0), blockTextureUVs[frontTex].u1, blockTextureUVs[frontTex].v1, diff, diff);
    */
}

void DrawCubeSprite(GLfloat x, GLfloat y, int blockType, float scale)
{
    /*int frontTex, topTex;
    GLuint texunit = Blocks[blockType].topTexUnit;

    if (texunit == 1.0){
        for (int i = 0; i < 8; i+=2){
            cubeSpriteVerts[i] = flatSpriteVertices[i]*scale+x;
            cubeSpriteVerts[i+1] = flatSpriteVertices[i+1]*scale+y;
        }
    }else{
        for (int i = 0; i < 24; i+=2){
            cubeSpriteVerts[i] = cubeSpriteVertices[i]*scale+x;
            cubeSpriteVerts[i+1] = cubeSpriteVertices[i+1]*scale+y;
        }
    }
    frontTex = Blocks[blockType].pxTex;
    topTex = Blocks[blockType].pyTex;
    cubeSpriteUVs[8] = cubeSpriteUVs[0] = blockTextureUVs[frontTex].u1;
    cubeSpriteUVs[9] = cubeSpriteUVs[1] = blockTextureUVs[frontTex].v1;
    cubeSpriteUVs[10] = cubeSpriteUVs[2] = blockTextureUVs[frontTex].u2;
    cubeSpriteUVs[11] = cubeSpriteUVs[3] = blockTextureUVs[frontTex].v2;
    cubeSpriteUVs[12] = cubeSpriteUVs[4] = blockTextureUVs[frontTex].u3;
    cubeSpriteUVs[13] = cubeSpriteUVs[5] = blockTextureUVs[frontTex].v3;
    cubeSpriteUVs[14] = cubeSpriteUVs[6] = blockTextureUVs[frontTex].u4;
    cubeSpriteUVs[15] = cubeSpriteUVs[7] = blockTextureUVs[frontTex].v4;
    cubeSpriteUVs[16] = blockTextureUVs[topTex].u1;
    cubeSpriteUVs[17] = blockTextureUVs[topTex].v1;
    cubeSpriteUVs[18] = blockTextureUVs[topTex].u2;
    cubeSpriteUVs[19] = blockTextureUVs[topTex].v2;
    cubeSpriteUVs[20] = blockTextureUVs[topTex].u3;
    cubeSpriteUVs[21] = blockTextureUVs[topTex].v3;
    cubeSpriteUVs[22] = blockTextureUVs[topTex].u4;
    cubeSpriteUVs[23] = blockTextureUVs[topTex].v4;

    if (texunit == 1.0){
        DrawImage2D(cubeSpriteVerts, 8*sizeof(GLfloat), cubeSpriteUVs, 8*sizeof(GLfloat), boxDrawIndices, sizeof(boxDrawIndices), blockPacks[1].textureInfo.ID, glm::vec4(1.0));
    }else if (blockType == DIRTGRASS){
        DrawCubeSpriteImage2D(cubeSpriteVerts, sizeof(cubeSpriteVerts), cubeSpriteUVs, sizeof(cubeSpriteUVs), cubeSpriteDrawIndices, sizeof(cubeSpriteDrawIndices), blockPacks[0].textureInfo.ID, glm::vec4(28.0f/255.0f, 50.0f/255.0f, 0.0, 1.0));
    }else{
        DrawCubeSpriteImage2D(cubeSpriteVerts, sizeof(cubeSpriteVerts), cubeSpriteUVs, sizeof(cubeSpriteUVs), cubeSpriteDrawIndices, sizeof(cubeSpriteDrawIndices), blockPacks[0].textureInfo.ID, glm::vec4(((float)Blocks[blockType].tr/255.0f), ((float)Blocks[blockType].tg/255.0f), ((float)Blocks[blockType].tb/255.0f), 1.0));
    }*/
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

    textureShader.Bind();
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

        // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sunTexture.ID);
    // Set our "myTextureSampler" sampler to user Texture Unit 0
    glUniform1i(textureShader.texID, 0);

    glUniformMatrix4fv(textureShader.mvpID, 1, GL_FALSE, &MVP[0][0]);

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

    textureShader.UnBind();
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);

    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
}

void DrawStars(float theta, glm::mat4 &MVP)
{
    glDisable(GL_CULL_FACE);
    textureShader.Bind();

        // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    // Set our "myTextureSampler" sampler to user Texture Unit 0
    glUniform1i(textureShader.texID, 0);

    glUniformMatrix4fv(textureShader.mvpID, 1, GL_FALSE, &MVP[0][0]);

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

    textureShader.UnBind();

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

    basicColorShader.Bind();
    glUniformMatrix4fv(basicColorShader.mvpID, 1, GL_FALSE, &MVP[0][0]);
    glUniform4f(basicColorShader.colorID, (GLfloat)color.r, (GLfloat)color.g, (GLfloat)color.b, (GLfloat)color.a);

    glBindBuffer(GL_ARRAY_BUFFER, gridvboID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gridelID);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glLineWidth(lineWidth);

    glDisable(GL_CULL_FACE);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, (void *)0);
    glEnable(GL_CULL_FACE);

    basicColorShader.UnBind();

    GlobalModelMatrix[0][0] = 1.0;
    GlobalModelMatrix[1][1] = 1.0;
    GlobalModelMatrix[2][2] = 1.0;
}

void DrawLoadingScreen(string text, bool clearColor, glm::vec4 backColor, int fontSize)
{
    cout << text << endl;
    fflush(stdout);

    glClearDepth(1.0);
    if (clearColor){
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    else{
        glClear(GL_DEPTH_BUFFER_BIT);
    }
    

    DrawFullScreenQuad(backColor);

    PrintText(text.c_str(), screenWidth2d / 2, screenHeight2d / 2, fontSize, 0, 1);

    SDL_GL_SwapWindow(mainWindow);

    glClear(GL_COLOR_BUFFER_BIT); //always clear color after in case next loading screen doesnt clear
}

//TODO: Delete this and just use the chunkmesher function
inline void MakeFace(BlockVertex *Verts, int vertexOffset, int waveEffect, int index, int tex, GLubyte color[3], GLubyte overlayColor[3])
{
    GLubyte texUnit = (GLubyte)(tex / 256);
    tex %= 256;

    Verts[index].color[0] = color[0];
    Verts[index].color[1] = color[1];
    Verts[index].color[2] = color[2];
    Verts[index + 1].color[0] = color[0];
    Verts[index + 1].color[1] = color[1];
    Verts[index + 1].color[2] = color[2];
    Verts[index + 2].color[0] = color[0];
    Verts[index + 2].color[1] = color[1];
    Verts[index + 2].color[2] = color[2];
    Verts[index + 3].color[0] = color[0];
    Verts[index + 3].color[1] = color[1];
    Verts[index + 3].color[2] = color[2];

    Verts[index].lampColor[0] = 255;
    Verts[index].lampColor[1] = 255;
    Verts[index].lampColor[2] = 255;
    Verts[index + 1].lampColor[0] = 255;
    Verts[index + 1].lampColor[1] = 255;
    Verts[index + 1].lampColor[2] = 255;
    Verts[index + 2].lampColor[0] = 255;
    Verts[index + 2].lampColor[1] = 255;
    Verts[index + 2].lampColor[2] = 255;
    Verts[index + 3].lampColor[0] = 255;

    Verts[index].sunLight = 0;
    Verts[index + 1].sunLight = 0;
    Verts[index + 2].sunLight = 0;
    Verts[index + 3].sunLight = 0;

    Verts[index].position.x = physicsBlockVertices[vertexOffset];
    Verts[index].position.y = physicsBlockVertices[vertexOffset + 1];
    Verts[index].position.z = physicsBlockVertices[vertexOffset + 2];
    Verts[index + 1].position.x = physicsBlockVertices[vertexOffset + 3];
    Verts[index + 1].position.y = physicsBlockVertices[vertexOffset + 4];
    Verts[index + 1].position.z = physicsBlockVertices[vertexOffset + 5];
    Verts[index + 2].position.x = physicsBlockVertices[vertexOffset + 6];
    Verts[index + 2].position.y = physicsBlockVertices[vertexOffset + 7];
    Verts[index + 2].position.z = physicsBlockVertices[vertexOffset + 8];
    Verts[index + 3].position.x = physicsBlockVertices[vertexOffset + 9];
    Verts[index + 3].position.y = physicsBlockVertices[vertexOffset + 10];
    Verts[index + 3].position.z = physicsBlockVertices[vertexOffset + 11];

    Verts[index].normal[0] = cubeNormals[vertexOffset];
    Verts[index].normal[1] = cubeNormals[vertexOffset + 1];
    Verts[index].normal[2] = cubeNormals[vertexOffset + 2];
    Verts[index + 1].normal[0] = cubeNormals[vertexOffset + 3];
    Verts[index + 1].normal[1] = cubeNormals[vertexOffset + 4];
    Verts[index + 1].normal[2] = cubeNormals[vertexOffset + 5];
    Verts[index + 2].normal[0] = cubeNormals[vertexOffset + 6];
    Verts[index + 2].normal[1] = cubeNormals[vertexOffset + 7];
    Verts[index + 2].normal[2] = cubeNormals[vertexOffset + 8];
    Verts[index + 3].normal[0] = cubeNormals[vertexOffset + 9];
    Verts[index + 3].normal[1] = cubeNormals[vertexOffset + 10];
    Verts[index + 3].normal[2] = cubeNormals[vertexOffset + 11];

    if (waveEffect == 2){
        Verts[index].color[3] = 255;
        Verts[index + 1].color[3] = 255;
        Verts[index + 2].color[3] = 255;
        Verts[index + 3].color[3] = 255;
    }
    else if (waveEffect == 1){
        Verts[index].color[3] = 255;
        Verts[index + 1].color[3] = 0;
        Verts[index + 2].color[3] = 0;
        Verts[index + 3].color[3] = 255;
    }
    else{
        Verts[index].color[3] = 0;
        Verts[index + 1].color[3] = 0;
        Verts[index + 2].color[3] = 0;
        Verts[index + 3].color[3] = 0;
    }

    Verts[index].tex[0] = 0.0f;
    Verts[index].tex[1] = 1.0f;
    Verts[index + 1].tex[0] = 0.0f;
    Verts[index + 1].tex[1] = 0.0f;
    Verts[index + 2].tex[0] = 1.0f;
    Verts[index + 2].tex[1] = 0.0f;
    Verts[index + 3].tex[0] = 1.0f;
    Verts[index + 3].tex[1] = 1.0f;
    Verts[index].textureIndex = (GLubyte)tex;
    Verts[index + 1].textureIndex = (GLubyte)tex;
    Verts[index + 2].textureIndex = (GLubyte)tex;
    Verts[index + 3].textureIndex = (GLubyte)tex;

    Verts[index].textureAtlas = (GLubyte)texUnit;
    Verts[index + 1].textureAtlas = (GLubyte)texUnit;
    Verts[index + 2].textureAtlas = (GLubyte)texUnit;
    Verts[index + 3].textureAtlas = (GLubyte)texUnit;
}

GLuint MakeBlockVbo(Block *block){

    vector <BlockVertex> vertices;
    vertices.resize(24);
    int btype = block->ID;

    GLubyte color[3], overlayColor[3];
    
    color[0] = Blocks[btype].color[0];
    color[1] = Blocks[btype].color[1];
    color[2] = Blocks[btype].color[2];
    overlayColor[0] = Blocks[btype].overlayColor[0];
    overlayColor[1] = Blocks[btype].overlayColor[1];
    overlayColor[2] = Blocks[btype].overlayColor[2];
    
    int index = 0;
    //front
    MakeFace(&(vertices[0]), 0, block->waveEffect, index, block->pzTex, color, overlayColor);
    index += 4;
    //right
    MakeFace(&(vertices[0]), 12, block->waveEffect, index, block->pxTex, color, overlayColor);
    index += 4;
    //top
    MakeFace(&(vertices[0]), 24, block->waveEffect, index, block->pyTex, color, overlayColor);
    index += 4;
    //left
    MakeFace(&(vertices[0]), 36, block->waveEffect, index, block->nxTex, color, overlayColor);
    index += 4;
    //bottom
    MakeFace(&(vertices[0]), 48, block->waveEffect, index, block->nyTex, color, overlayColor);
    index += 4;
    //back
    MakeFace(&(vertices[0]), 60, block->waveEffect, index, block->nzTex, color, overlayColor);
    index += 4;

    GLuint vboID;
    glGenBuffers(1, &vboID); // Create the buffer ID
    glBindBuffer(GL_ARRAY_BUFFER, vboID); // Bind the buffer (vertex array data)
    glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(BlockVertex), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 24 * sizeof(BlockVertex), &(vertices[0]));
    return vboID;
}

void Draw3DCube(Block *block, double x, double y, double z, glm::mat4 &VP, glm::mat4 &rotation){

    blockShader.Bind();
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);

    //    GLuint beamWidthID = GetUniform(blockShader, "beamWidth");

    glUniform1f(blockShader.lightTypeID, 0);

    float eyeDir[3] = { 0.0f, 0.0f, -1.0f };
    glUniform3fv(blockShader.eyeVecID, 1, eyeDir);
    glUniform1f(blockShader.fogEndID, (GLfloat)100000.0f);
    glUniform1f(blockShader.fogStartID, (GLfloat)10000.0f);
    glUniform3fv(blockShader.fogColorID, 1, eyeDir);
    glm::vec3 light = glm::vec3(glm::inverse(rotation) * glm::vec4(glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f)), 1.0));
    glUniform3f(blockShader.lightID, light.x, light.y, light.z);
    glUniform1f(blockShader.specularExponentID, graphicsOptions.specularExponent);
    glUniform1f(blockShader.specularIntensityID, graphicsOptions.specularIntensity*0.3);

    bindBlockPacks();

    glUniform1f(blockShader.blockDtID, (GLfloat)bdt);

    glUniform1f(blockShader.sunValID, 1.0f);

    glUniform1f(blockShader.alphaMultID, 1.0f);

    float blockAmbient = 0.000f;
    glUniform3f(blockShader.ambientID, blockAmbient, blockAmbient, blockAmbient);
    glUniform3f(blockShader.lightColorID, (GLfloat)1.0f, (GLfloat)1.0f, (GLfloat)1.0f);

    float fadeDist;
    fadeDist = (GLfloat)10000.0f;

    GLuint vboID = MakeBlockVbo(block);

    glUniform1f(blockShader.fadeDistanceID, fadeDist);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Chunk::vboIndicesID);

    glDisable(GL_CULL_FACE);

    GlobalModelMatrix[3][0] = (x);
    GlobalModelMatrix[3][1] = (y);
    GlobalModelMatrix[3][2] = (z);

    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glm::mat4 M = GlobalModelMatrix * rotation;
    glm::mat4 MVP = VP * M;

    glUniformMatrix4fv(blockShader.mvpID, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(blockShader.mID, 1, GL_FALSE, &M[0][0]);


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BlockVertex), 0);
    //UVs
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(BlockVertex), ((char *)NULL + (12)));
    //colors
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(BlockVertex), ((char *)NULL + (20)));
    //textureLocation, light, sunlight, texture unit
    glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(BlockVertex), ((char *)NULL + (24)));
    //normal
    glVertexAttribPointer(4, 3, GL_BYTE, GL_TRUE, sizeof(BlockVertex), ((char *)NULL + (28)));

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    

    glEnable(GL_CULL_FACE);

    glDeleteBuffers(1, &vboID);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);
    blockShader.UnBind();
}