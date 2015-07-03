#include "stdafx.h"
#include "ChunkRenderer.h"

#include "Camera.h"
#include "Chunk.h"
#include "ChunkMeshManager.h"
#include "Frustum.h"
#include "GameManager.h"
#include "GameRenderParams.h"
#include "GeometrySorter.h"
#include "PhysicsBlocks.h"
#include "RenderUtils.h"
#include "ShaderLoader.h"
#include "SoaOptions.h"
#include "soaUtils.h"

volatile f32 ChunkRenderer::fadeDist = 1.0f;
f32m4 ChunkRenderer::worldMatrix = f32m4(1.0f);

VGIndexBuffer ChunkRenderer::sharedIBO = 0;

void ChunkRenderer::init() {
    // Not thread safe
    if (!sharedIBO) { // Create shared IBO if needed
        std::vector<ui32> indices;
        const int NUM_INDICES = 589824;
        indices.resize(NUM_INDICES);

        ui32 j = 0u;
        for (size_t i = 0; i < indices.size() - 12u; i += 6u) {
            indices[i] = j;
            indices[i + 1] = j + 1u;
            indices[i + 2] = j + 2u;
            indices[i + 3] = j + 2u;
            indices[i + 4] = j + 3u;
            indices[i + 5] = j;
            j += 4u;
        }

        glGenBuffers(1, &sharedIBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sharedIBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, NUM_INDICES * sizeof(ui32), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, NUM_INDICES * sizeof(ui32), indices.data()); //arbitrarily set to 300000
    }

    { // Opaque
        m_opaqueProgram = ShaderLoader::createProgramFromFile("Shaders/BlockShading/standardShading.vert",
                                                              "Shaders/BlockShading/standardShading.frag");
        m_opaqueProgram.use();
        glUniform1i(m_opaqueProgram.getUniform("unTextures"), 0);
    }
    { // Transparent
        m_transparentProgram = ShaderLoader::createProgramFromFile("Shaders/BlockShading/standardShading.vert",
                                                                   "Shaders/BlockShading/cutoutShading.frag");
    }
    { // Cutout
        m_cutoutProgram = ShaderLoader::createProgramFromFile("Shaders/BlockShading/standardShading.vert",
                                                              "Shaders/BlockShading/cutoutShading.frag");
    }
    { // Water
        m_waterProgram = ShaderLoader::createProgramFromFile("Shaders/WaterShading/WaterShading.vert",
                                                             "Shaders/WaterShading/WaterShading.frag");
    }
    vg::GLProgram::unuse();
}

void ChunkRenderer::dispose() {
    if (m_opaqueProgram.isCreated()) m_opaqueProgram.dispose();
    if (m_transparentProgram.isCreated()) m_transparentProgram.dispose();
    if (m_cutoutProgram.isCreated()) m_cutoutProgram.dispose();
    if (m_waterProgram.isCreated()) m_waterProgram.dispose();
}

void ChunkRenderer::beginOpaque(VGTexture textureAtlas, const f32v3& sunDir, const f32v3& lightColor /*= f32v3(1.0f)*/, const f32v3& ambient /*= f32v3(0.0f)*/) {
    m_opaqueProgram.use();
    glUniform3fv(m_opaqueProgram.getUniform("unLightDirWorld"), 1, &(sunDir[0]));
    glUniform1f(m_opaqueProgram.getUniform("unSpecularExponent"), soaOptions.get(OPT_SPECULAR_EXPONENT).value.f);
    glUniform1f(m_opaqueProgram.getUniform("unSpecularIntensity"), soaOptions.get(OPT_SPECULAR_INTENSITY).value.f * 0.3f);

    // Bind the block textures
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(m_opaqueProgram.getUniform("unTextures"), 0); // TODO(Ben): Temporary
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureAtlas);

    f32 blockAmbient = 0.000f;
    glUniform3fv(m_opaqueProgram.getUniform("unAmbientLight"), 1, &ambient[0]);
    glUniform3fv(m_opaqueProgram.getUniform("unSunColor"), 1, &sunDir[0]);

    glUniform1f(m_opaqueProgram.getUniform("unFadeDist"), 100000.0f/*ChunkRenderer::fadeDist*/);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sharedIBO);
}

void ChunkRenderer::drawOpaque(const ChunkMesh *cm, const f64v3 &PlayerPos, const f32m4 &VP) const {
    setMatrixTranslation(worldMatrix, f64v3(cm->position), PlayerPos);

    f32m4 MVP = VP * worldMatrix;

    glUniformMatrix4fv(m_opaqueProgram.getUniform("unWVP"), 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(m_opaqueProgram.getUniform("unW"), 1, GL_FALSE, &worldMatrix[0][0]);

    glBindVertexArray(cm->vaoID);

    const ChunkMeshRenderData& chunkMeshInfo = cm->renderData;
    //top
    if (chunkMeshInfo.pyVboSize && PlayerPos.y > cm->position.y + chunkMeshInfo.lowestY) {
        glDrawElements(GL_TRIANGLES, chunkMeshInfo.pyVboSize, GL_UNSIGNED_INT, (void*)(chunkMeshInfo.pyVboOff * sizeof(GLuint)));
    }
    //front
    if (chunkMeshInfo.pzVboSize && PlayerPos.z > cm->position.z + chunkMeshInfo.lowestZ){
        glDrawElements(GL_TRIANGLES, chunkMeshInfo.pzVboSize, GL_UNSIGNED_INT, (void*)(chunkMeshInfo.pzVboOff * sizeof(GLuint)));
    }
    //back
    if (chunkMeshInfo.nzVboSize && PlayerPos.z < cm->position.z + chunkMeshInfo.highestZ){
        glDrawElements(GL_TRIANGLES, chunkMeshInfo.nzVboSize, GL_UNSIGNED_INT, (void*)(chunkMeshInfo.nzVboOff * sizeof(GLuint)));
    }
    //left
    if (chunkMeshInfo.nxVboSize && PlayerPos.x < cm->position.x + chunkMeshInfo.highestX){
        glDrawElements(GL_TRIANGLES, chunkMeshInfo.nxVboSize, GL_UNSIGNED_INT, (void*)(chunkMeshInfo.nxVboOff * sizeof(GLuint)));
    }
    //right
    if (chunkMeshInfo.pxVboSize && PlayerPos.x > cm->position.x + chunkMeshInfo.lowestX){
        glDrawElements(GL_TRIANGLES, chunkMeshInfo.pxVboSize, GL_UNSIGNED_INT, (void*)(chunkMeshInfo.pxVboOff * sizeof(GLuint)));
    }
    //bottom
    if (chunkMeshInfo.nyVboSize && PlayerPos.y < cm->position.y + chunkMeshInfo.highestY){
        glDrawElements(GL_TRIANGLES, chunkMeshInfo.nyVboSize, GL_UNSIGNED_INT, (void*)(chunkMeshInfo.nyVboOff * sizeof(GLuint)));
    }

    glBindVertexArray(0);
}

void ChunkRenderer::drawOpaqueCustom(const ChunkMesh* cm, vg::GLProgram& m_program, const f64v3& PlayerPos, const f32m4& VP) {
    setMatrixTranslation(worldMatrix, f64v3(cm->position), PlayerPos);

    f32m4 MVP = VP * worldMatrix;

    glUniformMatrix4fv(m_program.getUniform("unWVP"), 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(m_program.getUniform("unW"), 1, GL_FALSE, &worldMatrix[0][0]);

    glBindVertexArray(cm->vaoID);

    const ChunkMeshRenderData& chunkMeshInfo = cm->renderData;
    //top
    if (chunkMeshInfo.pyVboSize && PlayerPos.y > cm->position.y + chunkMeshInfo.lowestY) {
        glDrawElements(GL_TRIANGLES, chunkMeshInfo.pyVboSize, GL_UNSIGNED_INT, (void*)(chunkMeshInfo.pyVboOff * sizeof(GLuint)));
    }
    //front
    if (chunkMeshInfo.pzVboSize && PlayerPos.z > cm->position.z + chunkMeshInfo.lowestZ) {
        glDrawElements(GL_TRIANGLES, chunkMeshInfo.pzVboSize, GL_UNSIGNED_INT, (void*)(chunkMeshInfo.pzVboOff * sizeof(GLuint)));
    }
    //back
    if (chunkMeshInfo.nzVboSize && PlayerPos.z < cm->position.z + chunkMeshInfo.highestZ) {
        glDrawElements(GL_TRIANGLES, chunkMeshInfo.nzVboSize, GL_UNSIGNED_INT, (void*)(chunkMeshInfo.nzVboOff * sizeof(GLuint)));
    }
    //left
    if (chunkMeshInfo.nxVboSize && PlayerPos.x < cm->position.x + chunkMeshInfo.highestX) {
        glDrawElements(GL_TRIANGLES, chunkMeshInfo.nxVboSize, GL_UNSIGNED_INT, (void*)(chunkMeshInfo.nxVboOff * sizeof(GLuint)));
    }
    //right
    if (chunkMeshInfo.pxVboSize && PlayerPos.x > cm->position.x + chunkMeshInfo.lowestX) {
        glDrawElements(GL_TRIANGLES, chunkMeshInfo.pxVboSize, GL_UNSIGNED_INT, (void*)(chunkMeshInfo.pxVboOff * sizeof(GLuint)));
    }
    //bottom
    if (chunkMeshInfo.nyVboSize && PlayerPos.y < cm->position.y + chunkMeshInfo.highestY) {
        glDrawElements(GL_TRIANGLES, chunkMeshInfo.nyVboSize, GL_UNSIGNED_INT, (void*)(chunkMeshInfo.nyVboOff * sizeof(GLuint)));
    }

    glBindVertexArray(0);
}


void ChunkRenderer::beginTransparent(VGTexture textureAtlas, const f32v3& sunDir, const f32v3& lightColor /*= f32v3(1.0f)*/, const f32v3& ambient /*= f32v3(0.0f)*/) {
    m_transparentProgram.use();
    
    glUniform3fv(m_transparentProgram.getUniform("unLightDirWorld"), 1, &(sunDir[0]));
    glUniform1f(m_transparentProgram.getUniform("unSpecularExponent"), soaOptions.get(OPT_SPECULAR_EXPONENT).value.f);
    glUniform1f(m_transparentProgram.getUniform("unSpecularIntensity"), soaOptions.get(OPT_SPECULAR_INTENSITY).value.f * 0.3f);

    // Bind the block textures
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(m_transparentProgram.getUniform("unTextures"), 0); // TODO(Ben): Temporary
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureAtlas);

    f32 blockAmbient = 0.000f;
    glUniform3fv(m_transparentProgram.getUniform("unAmbientLight"), 1, &ambient[0]);
    glUniform3fv(m_transparentProgram.getUniform("unSunColor"), 1, &sunDir[0]);

    glUniform1f(m_transparentProgram.getUniform("unFadeDist"), 100000.0f/*ChunkRenderer::fadeDist*/);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sharedIBO);
}

void ChunkRenderer::drawTransparent(const ChunkMesh *cm, const f64v3 &playerPos, const f32m4 &VP) const {
    if (cm->transVaoID == 0) return;

    setMatrixTranslation(worldMatrix, f64v3(cm->position), playerPos);

    f32m4 MVP = VP * worldMatrix;

    glUniformMatrix4fv(m_transparentProgram.getUniform("MVP"), 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(m_transparentProgram.getUniform("M"), 1, GL_FALSE, &worldMatrix[0][0]);

    glBindVertexArray(cm->transVaoID);

    glDrawElements(GL_TRIANGLES, cm->renderData.transVboSize, GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);
}

void ChunkRenderer::beginCutout(VGTexture textureAtlas, const f32v3& sunDir, const f32v3& lightColor /*= f32v3(1.0f)*/, const f32v3& ambient /*= f32v3(0.0f)*/) {
    m_cutoutProgram.use();
    glUniform3fv(m_cutoutProgram.getUniform("unLightDirWorld"), 1, &(sunDir[0]));
    glUniform1f(m_cutoutProgram.getUniform("unSpecularExponent"), soaOptions.get(OPT_SPECULAR_EXPONENT).value.f);
    glUniform1f(m_cutoutProgram.getUniform("unSpecularIntensity"), soaOptions.get(OPT_SPECULAR_INTENSITY).value.f * 0.3f);

    // Bind the block textures
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(m_cutoutProgram.getUniform("unTextures"), 0); // TODO(Ben): Temporary
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureAtlas);

    f32 blockAmbient = 0.000f;
    glUniform3fv(m_cutoutProgram.getUniform("unAmbientLight"), 1, &ambient[0]);
    glUniform3fv(m_cutoutProgram.getUniform("unSunColor"), 1, &sunDir[0]);

    glUniform1f(m_cutoutProgram.getUniform("unFadeDist"), 100000.0f/*ChunkRenderer::fadeDist*/);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sharedIBO);
}

void ChunkRenderer::drawCutout(const ChunkMesh *cm, const f64v3 &playerPos, const f32m4 &VP) const {
    if (cm->cutoutVaoID == 0) return;

    setMatrixTranslation(worldMatrix, f64v3(cm->position), playerPos);

    f32m4 MVP = VP * worldMatrix;

    glUniformMatrix4fv(m_cutoutProgram.getUniform("MVP"), 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(m_cutoutProgram.getUniform("M"), 1, GL_FALSE, &worldMatrix[0][0]);

    glBindVertexArray(cm->cutoutVaoID);

    glDrawElements(GL_TRIANGLES, cm->renderData.cutoutVboSize, GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);

}

void ChunkRenderer::beginLiquid(VGTexture textureAtlas, const f32v3& sunDir, const f32v3& lightColor /*= f32v3(1.0f)*/, const f32v3& ambient /*= f32v3(0.0f)*/) {
    m_waterProgram.use();
    glUniform3fv(m_waterProgram.getUniform("unLightDirWorld"), 1, &(sunDir[0]));
    glUniform1f(m_waterProgram.getUniform("unSpecularExponent"), soaOptions.get(OPT_SPECULAR_EXPONENT).value.f);
    glUniform1f(m_waterProgram.getUniform("unSpecularIntensity"), soaOptions.get(OPT_SPECULAR_INTENSITY).value.f * 0.3f);

    // Bind the block textures
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(m_waterProgram.getUniform("unTextures"), 0); // TODO(Ben): Temporary
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureAtlas);

    f32 blockAmbient = 0.000f;
    glUniform3fv(m_waterProgram.getUniform("unAmbientLight"), 1, &ambient[0]);
    glUniform3fv(m_waterProgram.getUniform("unSunColor"), 1, &sunDir[0]);

    glUniform1f(m_waterProgram.getUniform("unFadeDist"), 100000.0f/*ChunkRenderer::fadeDist*/);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sharedIBO);
}

void ChunkRenderer::drawLiquid(const ChunkMesh *cm, const f64v3 &PlayerPos, const f32m4 &VP) const {
    //use drawWater bool to avoid checking frustum twice
    if (cm->inFrustum && cm->waterVboID){

        setMatrixTranslation(worldMatrix, f64v3(cm->position), PlayerPos);

        f32m4 MVP = VP * worldMatrix;

        glUniformMatrix4fv(m_waterProgram.getUniform("MVP"), 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(m_waterProgram.getUniform("M"), 1, GL_FALSE, &worldMatrix[0][0]);

        glBindVertexArray(cm->waterVaoID);

        glDrawElements(GL_TRIANGLES, cm->renderData.waterIndexSize, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
    }
}

void ChunkRenderer::end() {
    vg::GLProgram::unuse();
}
