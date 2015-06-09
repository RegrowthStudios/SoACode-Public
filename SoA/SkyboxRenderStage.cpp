#include "stdafx.h"
#include "SkyboxRenderStage.h"

#include <GL/glew.h>
#include <Vorb/graphics/DepthState.h>
#include <Vorb/io/IOManager.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"
#include "Errors.h"
#include "LoadContext.h"
#include "ModPathResolver.h"
#include "ShaderLoader.h"
#include "SkyboxRenderer.h"
#include "SoAState.h"

const ui32 TASK_WORK = 4;
const ui32 TOTAL_TASKS = 8;
const ui32 TOTAL_WORK = TOTAL_TASKS * TASK_WORK;

void SkyboxRenderStage::init(vui::GameWindow* window, StaticLoadContext& context) {
    IRenderStage::init(window, context);
    context.addAnticipatedWork(TOTAL_WORK, TOTAL_TASKS);
}

void SkyboxRenderStage::hook(SoaState* state) {
    m_textureResolver = &state->texturePathResolver;
}

void SkyboxRenderStage::load(StaticLoadContext& context) {
    // Create texture array
    context.addTask([&](Sender, void*) {
        glGenTextures(1, &m_skyboxTextureArray);
        m_skyboxRenderer.initGL();
        context.addWorkCompleted(TASK_WORK);
    }, false);

    // TODO(Ben): Error check for dimensions
    // Front
    context.addTask([&](Sender, void*) {
        vio::Path path;
        m_textureResolver->resolvePath("Sky/Skybox/front.png", path);
        vg::ScopedBitmapResource frontRes = vg::ImageIO().load(path);
        if (frontRes.data == nullptr) pError("Failed to load Sky/Skybox/front.png");
        glBindTexture(GL_TEXTURE_2D_ARRAY, m_skyboxTextureArray);
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, frontRes.width, frontRes.width, 1, GL_RGBA, GL_UNSIGNED_BYTE, frontRes.data);
        context.addWorkCompleted(TASK_WORK);
    }, false);
    // Right
    context.addTask([&](Sender, void*) {
        vio::Path path;
        m_textureResolver->resolvePath("Sky/Skybox/right.png", path);
        vg::ScopedBitmapResource rightRes = vg::ImageIO().load(path);
        if (rightRes.data == nullptr) pError("Failed to load Sky/Skybox/right.png");
        glBindTexture(GL_TEXTURE_2D_ARRAY, m_skyboxTextureArray);
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 1, rightRes.width, rightRes.width, 1, GL_RGBA, GL_UNSIGNED_BYTE, rightRes.data);
        context.addWorkCompleted(TASK_WORK);
    }, false);
    // Top
    context.addTask([&](Sender, void*) {
        vio::Path path;
        m_textureResolver->resolvePath("Sky/Skybox/top.png", path);
        vg::ScopedBitmapResource topRes = vg::ImageIO().load(path);
        if (topRes.data == nullptr) pError("Failed to load Sky/Skybox/top.png");
        glBindTexture(GL_TEXTURE_2D_ARRAY, m_skyboxTextureArray);
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 2, topRes.width, topRes.width, 1, GL_RGBA, GL_UNSIGNED_BYTE, topRes.data);
        context.addWorkCompleted(TASK_WORK);
    }, false);
    // Left
    context.addTask([&](Sender, void*) {
        vio::Path path;
        m_textureResolver->resolvePath("Sky/Skybox/left.png", path);
        vg::ScopedBitmapResource leftRes = vg::ImageIO().load(path);
        if (leftRes.data == nullptr) pError("Failed to load Sky/Skybox/left.png");
        glBindTexture(GL_TEXTURE_2D_ARRAY, m_skyboxTextureArray);
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 3, leftRes.width, leftRes.width, 1, GL_RGBA, GL_UNSIGNED_BYTE, leftRes.data);
        context.addWorkCompleted(TASK_WORK);
    }, false);
    // Bottom
    context.addTask([&](Sender, void*) {
        vio::Path path;
        m_textureResolver->resolvePath("Sky/Skybox/bottom.png", path);
        vg::ScopedBitmapResource botRes = vg::ImageIO().load(path);
        if (botRes.data == nullptr) pError("Failed to load Sky/Skybox/bottom.png");
        glBindTexture(GL_TEXTURE_2D_ARRAY, m_skyboxTextureArray);
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 4, botRes.width, botRes.width, 1, GL_RGBA, GL_UNSIGNED_BYTE, botRes.data);
        context.addWorkCompleted(TASK_WORK);
    }, false);
    // Back
    context.addTask([&](Sender, void*) {
        vio::Path path;
        m_textureResolver->resolvePath("Sky/Skybox/back.png", path);
        vg::ScopedBitmapResource backRes = vg::ImageIO().load(path);
        if (backRes.data == nullptr) pError("Failed to load Sky/Skybox/back.png");
        glBindTexture(GL_TEXTURE_2D_ARRAY, m_skyboxTextureArray);
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 5, backRes.width, backRes.width, 1, GL_RGBA, GL_UNSIGNED_BYTE, backRes.data);
        context.addWorkCompleted(TASK_WORK);
    }, false);

    // Tex parameters and mipmaps
    context.addTask([&](Sender, void*) {
        glBindTexture(GL_TEXTURE_2D_ARRAY, m_skyboxTextureArray);
        context.addWorkCompleted(TOTAL_WORK);

        // Set up tex parameters
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

        // Unbind
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

        // Check if we had any errors
        checkGlError("SkyboxRenderStage::load()");
        context.addWorkCompleted(TASK_WORK);
    }, false);
}

void SkyboxRenderStage::render(const Camera* camera) {

    // Check if FOV or Aspect Ratio changed
    if (m_fieldOfView != camera->getFieldOfView() ||
        m_aspectRatio != camera->getAspectRatio()) {
        updateProjectionMatrix(camera);
    }
    // Draw using custom proj and camera view
    drawSpace(m_projectionMatrix * camera->getViewMatrix());
}

void SkyboxRenderStage::loadSkyboxTexture() {
    // Set up the storage
    glGenTextures(1, &m_skyboxTextureArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_skyboxTextureArray);

   
  
    

    if (frontRes.width != frontRes.height) {
        pError("Skybox textures must have equal width and height!");
    }

    // Calculate max mipmap level
    int maxMipLevel = 0;
    int width = frontRes.width;
    while (width > 1) {
        width >>= 1;
        maxMipLevel++;
    }

    // Set up all the mimpap storage
    width = frontRes.width;
    for (i32 i = 0; i < maxMipLevel; i++) {
        glTexImage3D(GL_TEXTURE_2D_ARRAY, i, GL_RGBA8, width, width, 6, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        width >>= 1;
        if (width < 1) width = 1;
    }

    // Upload the data to VRAM
    
    
}

void SkyboxRenderStage::drawSpace(glm::mat4 &VP) {
    vg::DepthState::NONE.set();
    m_skyboxRenderer.drawSkybox(VP, m_skyboxTextureArray);
    vg::DepthState::FULL.set();
}

void SkyboxRenderStage::updateProjectionMatrix(const Camera* camera) {
    // Set the camera clipping plane for rendering the skybox and set the projection matrix
    // The clipping dimensions don't matter so long as the skybox fits inside them
    #define SKYBOX_ZNEAR 0.01f
    #define SKYBOX_ZFAR 300.0f

    m_fieldOfView = camera->getFieldOfView();
    m_aspectRatio = camera->getAspectRatio();

    // Set up projection matrix
    m_projectionMatrix = glm::perspective(m_fieldOfView, m_aspectRatio, SKYBOX_ZNEAR, SKYBOX_ZFAR);
}
