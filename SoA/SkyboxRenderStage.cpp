#include "stdafx.h"
#include "SkyboxRenderStage.h"

#include <GL/glew.h>
#include <Vorb/graphics/DepthState.h>
#include <Vorb/io/IOManager.h>

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
    m_textureResolver = &state->clientState.texturePathResolver;
}

void SkyboxRenderStage::load(StaticLoadContext& context) {
    // Create texture array
    context.addTask([&](Sender, void*) {
        glGenTextures(1, &m_skyboxTextureArray);
        m_skyboxRenderer.initGL();
        context.addWorkCompleted(TASK_WORK);
        checkGlError("SkyboxRenderStage inittt");
    }, false);

    // Front (also allocates storage)
    context.addTask([&](Sender, void*) {
        vio::Path path;
        m_textureResolver->resolvePath("Sky/Skybox/front.png", path);
        vg::ScopedBitmapResource frontRes = vg::ImageIO().load(path);
        m_resolution = frontRes.width;
        if (frontRes.height != m_resolution) {
            pError("Skybox textures must have equal width and height!");
        }
        if (frontRes.data == nullptr) pError("Failed to load Sky/Skybox/front.png");
        glBindTexture(GL_TEXTURE_2D_ARRAY, m_skyboxTextureArray);
        // Calculate max mipmap level
        int maxMipLevel = 0;
        int width = m_resolution;
        while (width > 1) {
            width >>= 1;
            maxMipLevel++;
        }
        // Set up all the storage
        width = m_resolution;
        for (i32 i = 0; i < maxMipLevel; i++) {
            glTexImage3D(GL_TEXTURE_2D_ARRAY, i, GL_RGBA8, width, width, 6, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
            width >>= 1;
            if (width < 1) width = 1;
        }
        // Set mipmap parameters
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LOD, maxMipLevel);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, maxMipLevel);
        // Upload data
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, frontRes.width, frontRes.height, 1, GL_RGBA, GL_UNSIGNED_BYTE, frontRes.data);
        context.addWorkCompleted(TASK_WORK);
        checkGlError("SkyboxRenderStage waaaa");
    }, false);
    // Right
    context.addTask([&](Sender, void*) {
        loadTexture("Sky/Skybox/right.png", 1);
        context.addWorkCompleted(TASK_WORK);
    }, false);
    // Top
    context.addTask([&](Sender, void*) {
        loadTexture("Sky/Skybox/top.png", 2);
        context.addWorkCompleted(TASK_WORK);
    }, false);
    // Left
    context.addTask([&](Sender, void*) {
        loadTexture("Sky/Skybox/left.png", 3);
        context.addWorkCompleted(TASK_WORK);
    }, false);
    // Bottom
    context.addTask([&](Sender, void*) {
        loadTexture("Sky/Skybox/bottom.png", 4);
        context.addWorkCompleted(TASK_WORK);
    }, false);
    // Back
    context.addTask([&](Sender, void*) {
        loadTexture("Sky/Skybox/back.png", 5);
        context.addWorkCompleted(TASK_WORK);
    }, false);

    // Tex parameters and mipmaps
    context.addTask([&](Sender, void*) {
        glBindTexture(GL_TEXTURE_2D_ARRAY, m_skyboxTextureArray);

        // Set up tex parameters
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

        // Unbind
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

        // Check if we had any errors
        checkGlError("SkyboxRenderStage mipmaps");
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


void SkyboxRenderStage::drawSpace(f32m4 &VP) {
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
    m_projectionMatrix = vmath::perspective(m_fieldOfView, m_aspectRatio, SKYBOX_ZNEAR, SKYBOX_ZFAR);
}

void SkyboxRenderStage::loadTexture(const char* relPath, int index) {
    vio::Path path;
    m_textureResolver->resolvePath(relPath, path);
    vg::ScopedBitmapResource res = vg::ImageIO().load(path);
    if (res.height != m_resolution || res.width != m_resolution) {
        pError("Skybox textures must all have equal width and height!");
    }
    if (res.data == nullptr) pError("Failed to load " + nString(relPath));
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_skyboxTextureArray);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, index, res.width, res.height, 1, GL_RGBA, GL_UNSIGNED_BYTE, res.data);  
    checkGlError("SkyboxRenderStage::load() " + nString(relPath));
}