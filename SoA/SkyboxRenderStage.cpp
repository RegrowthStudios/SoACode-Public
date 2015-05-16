#include "stdafx.h"
#include "SkyboxRenderStage.h"

#include <GL/glew.h>
#include <Vorb/graphics/DepthState.h>
#include <Vorb/io/IOManager.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"
#include "Errors.h"
#include "ModPathResolver.h"
#include "ShaderLoader.h"
#include "SkyboxRenderer.h"

SkyboxRenderStage::SkyboxRenderStage(const Camera* camera, const ModPathResolver* textureResolver) :
                                     IRenderStage("Skybox", camera),
                                     m_textureResolver(textureResolver) {

   updateProjectionMatrix();
}


SkyboxRenderStage::~SkyboxRenderStage() {
    // Empty
}

void SkyboxRenderStage::render() {

    // Lazy texture init
    if (m_skyboxTextureArray == 0) {
        loadSkyboxTexture();
    }

    // Check if FOV or Aspect Ratio changed
    if (m_fieldOfView != m_camera->getFieldOfView() ||
        m_aspectRatio != m_camera->getAspectRatio()) {
        updateProjectionMatrix();
    }
    // Draw using custom proj and camera view
    drawSpace(m_projectionMatrix * m_camera->getViewMatrix());
}

void SkyboxRenderStage::reloadShader() {
    m_skyboxRenderer.destroy();
}

void SkyboxRenderStage::loadSkyboxTexture() {
    // Set up the storage
    glGenTextures(1, &m_skyboxTextureArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_skyboxTextureArray);

    vio::Path path;
    // TODO(Ben): Error check for dimensions
    m_textureResolver->resolvePath("Sky/Skybox/front.png", path);
    vg::BitmapResource frontRes = vg::ImageIO().load(path);
    if (frontRes.data == nullptr) pError("Failed to load Sky/Skybox/front.png");
    m_textureResolver->resolvePath("Sky/Skybox/right.png", path);
    vg::BitmapResource rightRes = vg::ImageIO().load(path);
    if (rightRes.data == nullptr) pError("Failed to load Sky/Skybox/right.png");
    m_textureResolver->resolvePath("Sky/Skybox/top.png", path);
    vg::BitmapResource topRes = vg::ImageIO().load(path);
    if (topRes.data == nullptr) pError("Failed to load Sky/Skybox/top.png");
    m_textureResolver->resolvePath("Sky/Skybox/left.png", path);
    vg::BitmapResource leftRes = vg::ImageIO().load(path);
    if (leftRes.data == nullptr) pError("Failed to load Sky/Skybox/left.png");
    m_textureResolver->resolvePath("Sky/Skybox/bottom.png", path);
    vg::BitmapResource botRes = vg::ImageIO().load(path);
    if (botRes.data == nullptr) pError("Failed to load Sky/Skybox/bottom.png");
    m_textureResolver->resolvePath("Sky/Skybox/back.png", path);
    vg::BitmapResource backRes = vg::ImageIO().load(path);
    if (backRes.data == nullptr) pError("Failed to load Sky/Skybox/back.png");

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
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, frontRes.width, frontRes.width, 1, GL_RGBA, GL_UNSIGNED_BYTE, frontRes.data);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 1, rightRes.width, rightRes.width, 1, GL_RGBA, GL_UNSIGNED_BYTE, rightRes.data);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 2, topRes.width, topRes.width, 1, GL_RGBA, GL_UNSIGNED_BYTE, topRes.data);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 3, leftRes.width, leftRes.width, 1, GL_RGBA, GL_UNSIGNED_BYTE, leftRes.data);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 4, botRes.width, botRes.width, 1, GL_RGBA, GL_UNSIGNED_BYTE, botRes.data);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 5, backRes.width, backRes.width, 1, GL_RGBA, GL_UNSIGNED_BYTE, backRes.data);

    // Set up tex parameters
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, maxMipLevel);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LOD, maxMipLevel);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_GENERATE_MIPMAP, GL_TRUE);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    // Unbind
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    // Check if we had any errors
    checkGlError("SkyboxRenderStage::loadSkyboxTexture()"); 
}

void SkyboxRenderStage::drawSpace(glm::mat4 &VP) {
    vg::DepthState::NONE.set();
    m_skyboxRenderer.drawSkybox(VP, m_skyboxTextureArray);
    vg::DepthState::FULL.set();
}

void SkyboxRenderStage::updateProjectionMatrix() {
    // Set the camera clipping plane for rendering the skybox and set the projection matrix
    // The clipping dimensions don't matter so long as the skybox fits inside them
    #define SKYBOX_ZNEAR 0.01f
    #define SKYBOX_ZFAR 300.0f

    m_fieldOfView = m_camera->getFieldOfView();
    m_aspectRatio = m_camera->getAspectRatio();

    // Set up projection matrix
    m_projectionMatrix = glm::perspective(m_fieldOfView, m_aspectRatio, SKYBOX_ZNEAR, SKYBOX_ZFAR);
}
