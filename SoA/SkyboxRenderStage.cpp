#include "stdafx.h"
#include "SkyboxRenderStage.h"

#include <GL/glew.h>
#include <Vorb/graphics/DepthState.h>
#include <Vorb/graphics/ShaderManager.h>
#include <Vorb/io/IOManager.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"
#include "SkyboxRenderer.h"

SkyboxRenderStage::SkyboxRenderStage(const Camera* camera) :
                                     IRenderStage("Skybox", camera),
                                     m_skyboxRenderer(new SkyboxRenderer()) {

   updateProjectionMatrix();
}


SkyboxRenderStage::~SkyboxRenderStage() {
    delete m_skyboxRenderer;
}

void SkyboxRenderStage::render() {

    // Lazy shader init
    if (!m_program) {
        buildShaders();
    }

    // Check if FOV or Aspect Ratio changed
    if (m_fieldOfView != m_camera->getFieldOfView() ||
        m_aspectRatio != m_camera->getAspectRatio()) {
        updateProjectionMatrix();
    }
    // Draw using custom proj and camera view
    drawSpace(m_projectionMatrix * m_camera->getViewMatrix());
}

void SkyboxRenderStage::buildShaders() {
    vio::IOManager iom; // TODO(Ben): Maybe pass in instead
    m_program = vg::ShaderManager::createProgramFromFile("Shaders/TextureShading/TextureShading.vert",
                                                         "Shaders/TextureShading/TextureShading.frag");

    std::vector<nString> attribs;
    attribs.push_back("vPosition");
    m_program->setAttributes(attribs);

    m_program->link();
    m_program->initAttributes();
    m_program->initUniforms();
}

void SkyboxRenderStage::drawSpace(glm::mat4 &VP) {
    vg::DepthState::NONE.set();
    m_skyboxRenderer->drawSkybox(m_program, VP, skyboxTextures);
    vg::DepthState::FULL.set();
}

// Ben: This is terrible but I don't feel like fixing it since its temporary
void SkyboxRenderStage::drawSun(float theta, glm::mat4 &MVP) {
    double radius = 2800000.0;
    double size = 200000.0;
    float off = (float)atan(size / radius); // in radians
    float cosTheta = cos(theta - off);
    float sinTheta = sin(theta - off);
    float cosTheta2 = cos(theta + off);
    float sinTheta2 = sin(theta + off);

    // Bind shader
    m_program->use();
    m_program->enableVertexAttribArrays();

    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sunTexture.id);
    // Set our "myTextureSampler" sampler to user Texture Unit 0
    glUniform1i(m_program->getUniform("unTex"), 0);

    glUniformMatrix4fv(m_program->getUniform("unWVP"), 1, GL_FALSE, &MVP[0][0]);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    GLfloat sunUVs[8];
    GLfloat sunVerts[12];
    const GLushort sunIndices[6] = { 0, 1, 2, 2, 3, 0 };

    sunUVs[0] = 0;
    sunUVs[1] = 0;
    sunUVs[2] = 0;
    sunUVs[3] = 1;
    sunUVs[4] = 1;
    sunUVs[5] = 1;
    sunUVs[6] = 1;
    sunUVs[7] = 0;

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

    static GLuint vertexbuffer = 0;
    static GLuint uvbuffer = 0;

    if (vertexbuffer == 0) {
        glGenBuffers(1, &vertexbuffer);
        glGenBuffers(1, &uvbuffer);
    }

    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sunVerts), sunVerts, GL_STREAM_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sunUVs), sunUVs, GL_STREAM_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // 2nd attribute buffer : UVs
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, sunIndices);

    m_program->disableVertexAttribArrays();
    m_program->unuse();

    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
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
