#include "stdafx.h"
#include "WorldStructs.h"
#include "RenderTask.h"

#include <cstdlib>

#include "BlockData.h"
#include "Chunk.h"
#include "Options.h"
#include "GameManager.h"
#include "GLProgramManager.h"

#include "Texture2d.h"

MultiplePreciseTimer globalMultiplePreciseTimer;

class Item *ObjectList[OBJECT_LIST_SIZE];

Marker::Marker(const glm::dvec3 &Pos, string Name, glm::vec3 Color) : pos(Pos), name(Name), dist(0.0)
{
    //SetText(nameTex, name.c_str(), 0, 0, 35, 1);
}

void Marker::Draw(glm::mat4 &VP, const glm::dvec3 &playerPos)
{
 //   static double oldDist = 0.0f;
 //   dist = glm::length(pos - playerPos)*0.5;

 //   vcore::GLProgram* program = GameManager::glProgramManager->getProgram("FixedSizeBillboard");
 //   program->use();
 //   program->enableVertexAttribArrays();

 //   GlobalModelMatrix[3][0] = (0.0);
 //   GlobalModelMatrix[3][1] = (0.0);
 //   GlobalModelMatrix[3][2] = (0.0);

 //   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Chunk::vboIndicesID);
 //   glm::mat4 MVP = VP;
 //   glUniform2f(program->getUniform("UVstart"), 0.0f, 0.0f);
 //   float pixSize = 40.0f;
 //   float width = pixSize / screenWidth2d;
 //   float height = (pixSize * ((float)screenHeight2d / screenWidth2d)) / screenHeight2d;
 //   glUniform1f(program->getUniform("width"), width);
 //   glUniform1f(program->getUniform("height"), height);
 //   glUniform2f(program->getUniform("UVstart"), 1.0f, 1.0f);
 //   glUniform4f(program->getUniform("particleColor"), color.r, color.g, color.b, 0.5f);
 //   glUniform1f(program->getUniform("textureUnitID"), 0.0f);
 //   glUniform2f(program->getUniform("UVmod"), -0.5f, 0.0f);

 //   glUniformMatrix4fv(program->getUniform("MVP"), 1, GL_FALSE, &MVP[0][0]);
 //   glActiveTexture(GL_TEXTURE0);
 //   glBindTexture(GL_TEXTURE_2D, markerTexture.ID);

 //   GLuint bufferID;
 //   glGenBuffers(1, &bufferID);
 //   vector <FixedSizeBillboardVertex> billVerts;

 //   int n = 0;
 //   billVerts.resize(4);
 //   billVerts[n].pos = pos - playerPos;
 //   billVerts[n + 1].pos = pos - playerPos;
 //   billVerts[n + 2].pos = pos - playerPos;
 //   billVerts[n + 3].pos = pos - playerPos;
 //   billVerts[n].uv[0] = 255;
 //   billVerts[n].uv[1] = 255;
 //   billVerts[n + 1].uv[0] = 0;
 //   billVerts[n + 1].uv[1] = 255;
 //   billVerts[n + 2].uv[0] = 0;
 //   billVerts[n + 2].uv[1] = 0;
 //   billVerts[n + 3].uv[0] = 255;
 //   billVerts[n + 3].uv[1] = 0;
 //   glBindBuffer(GL_ARRAY_BUFFER, bufferID);
 //   glBufferData(GL_ARRAY_BUFFER, sizeof(FixedSizeBillboardVertex)* billVerts.size(), NULL, GL_STREAM_DRAW);
 //   glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(FixedSizeBillboardVertex)* billVerts.size(), &(billVerts[0]));
 //   glVertexAttribPointer(program->getAttribute("position"), 3, GL_FLOAT, GL_FALSE, sizeof(FixedSizeBillboardVertex), 0);
 //   glVertexAttribPointer(program->getAttribute("uv"), 2, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(FixedSizeBillboardVertex), (void *)12);

 //   glDisable(GL_DEPTH_TEST);
 //   glDepthMask(GL_FALSE);
 //   glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
 //   glDepthMask(GL_TRUE);
 //   glEnable(GL_DEPTH_TEST);
 //   glUniform4f(program->getUniform("particleColor"), color.r, color.g, color.b, 1.0f);
 //   glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

 //   program->disableVertexAttribArrays();
 //   program->unuse();

 //   char diststr[64];
 //   sprintf(diststr, "%.2lf km", dist*0.001);
 //   if (oldDist != dist){
 //   //    SetText(distText, diststr, 0, 0, 30, 1);
 //   }

 //   glDisable(GL_DEPTH_TEST);
 //   glDepthMask(GL_FALSE);
 ////   distText.DrawFixedSize3D(VP, playerPos, pos, glm::vec2(0.1, 0.0), glm::vec4(color.color.r, color.color.g, color.color.b, 0.5f));
 ////   nameTex.DrawFixedSize3D(VP, playerPos, pos, glm::vec2(-0.5, 1.0), glm::vec4(color.color.r, color.color.g, color.color.b, 0.5f));
 //   glDepthMask(GL_TRUE);
 //   glEnable(GL_DEPTH_TEST);
 // //  distText.DrawFixedSize3D(VP, playerPos, pos, glm::vec2(0.1, 0.0), glm::vec4(color.color.r, color.color.g, color.color.b, 1.0f));
 ////   nameTex.DrawFixedSize3D(VP, playerPos, pos, glm::vec2(-0.5, 1.0), glm::vec4(color.color.r, color.color.g, color.color.b, 1.0f));

 //   oldDist = dist;
 //   glDeleteBuffers(1, &bufferID);
}

Biome::Biome()
{
    memset(this, 0, sizeof(Biome)); //zero the memory
    maxHeight = 999999;
    maxHeightSlopeLength = 0;
    looseSoilDepth = 4;
    name = "NO BIOME";
    vecIndex = -1;
    surfaceBlock = DIRTGRASS;
    underwaterBlock = SAND;
    beachBlock = SAND;
    filename = "";

    //default layers
    int i = 0;
    for (; i < 3; i++) surfaceLayers[i] = DIRT;
    for (; i < 10; i++) surfaceLayers[i] = GRAVEL;
    for (; i < 24; i++) surfaceLayers[i] = SLATE;
    for (; i < 35; i++) surfaceLayers[i] = LIMESTONE;
    for (; i < 55; i++) surfaceLayers[i] = SLATE;
    for (; i < 80; i++) surfaceLayers[i] = GNEISS;
    for (; i < 120; i++) surfaceLayers[i] = BASALT;
    for (; i < 160; i++) surfaceLayers[i] = GRANITE;
    for (; i < SURFACE_DEPTH; i++) surfaceLayers[i] = STONE;
}