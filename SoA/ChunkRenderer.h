#pragma once
#include <vector>
#include "types.h"
#include "ChunkMesh.h"
#include "GLProgram.h"

class ChunkRenderer {
public:
    static void drawSonar(const std::vector <ChunkMesh *>& chunkMeshes, glm::mat4 &VP, glm::dvec3 &position);
    static void drawBlocks(const std::vector <ChunkMesh *>& chunkMeshes, const glm::mat4 &VP, const glm::dvec3 &position, glm::vec3 &lightPos, glm::vec3 &lightColor, GLfloat lightActive, GLfloat sunVal, GLfloat fogEnd, GLfloat fogStart, GLfloat *fogColor, const GLfloat *eyeDir);
    static void drawCutoutBlocks(const std::vector <ChunkMesh *>& chunkMeshes, const glm::mat4 &VP, const glm::dvec3 &position, glm::vec3 &lightPos, glm::vec3 &lightColor, GLfloat lightActive, GLfloat sunVal, GLfloat fogEnd, GLfloat fogStart, GLfloat *fogColor, const GLfloat *eyeDir);
    static void drawTransparentBlocks(const std::vector <ChunkMesh *>& chunkMeshes, const glm::mat4 &VP, const glm::dvec3 &position, glm::vec3 &lightPos, glm::vec3 &lightColor, GLfloat lightActive, GLfloat sunVal, GLfloat fogEnd, GLfloat fogStart, GLfloat *fogColor, const GLfloat *eyeDir);
    //static void drawPhysicsBlocks(glm::mat4 &VP, const glm::dvec3 &position, glm::vec3 &lightPos, glm::vec3 &lightColor, GLfloat lightActive, GLfloat sunVal, GLfloat fogEnd, GLfloat fogStart, GLfloat *fogColor, const GLfloat *eyeDir);
    static void drawWater(const std::vector <ChunkMesh *>& chunkMeshes, glm::mat4 &VP, const glm::dvec3 &position, GLfloat sunVal, GLfloat fogEnd, GLfloat fogStart, GLfloat *fogColor, glm::vec3 &lightPos, glm::vec3 &lightColor, bool underWater);

    static void bindTransparentVao(ChunkMesh *CMI);
    static void bindCutoutVao(ChunkMesh *CMI);
    static void bindVao(ChunkMesh *CMI);
    static void bindWaterVao(ChunkMesh *CMI);
   
private:
    static void drawChunkBlocks(const ChunkMesh *CMI, const vcore::GLProgram* program, const glm::dvec3 &PlayerPos, const glm::mat4 &VP);
    static void drawChunkTransparentBlocks(const ChunkMesh *CMI, const vcore::GLProgram* program, const glm::dvec3 &playerPos, const glm::mat4 &VP);
    static void drawChunkCutoutBlocks(const ChunkMesh *CMI, const vcore::GLProgram* program, const glm::dvec3 &playerPos, const glm::mat4 &VP);
    static void drawChunkWater(const ChunkMesh *CMI, const vcore::GLProgram* program, const glm::dvec3 &PlayerPos, const glm::mat4 &VP);

};