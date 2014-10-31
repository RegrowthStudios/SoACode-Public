#include "stdafx.h"
#include <GL/glew.h>
#include "SpaceRenderStage.h"
#include "SkyboxRenderer.h"
#include "Camera.h"

SpaceRenderStage::SpaceRenderStage(vg::GLProgram* glProgram,
                                   Camera* camera) :
    _skyboxRenderer(new SkyboxRenderer()),
    _glProgram(glProgram),
    _camera(camera)
{
}


SpaceRenderStage::~SpaceRenderStage() {
    delete _skyboxRenderer;
}

void SpaceRenderStage::draw() {
    // Set the camera clipping plane for rendering the skybox and update the projection matrix
#define SKYBOX_ZNEAR 1000000.0f
#define SKYBOX_ZFAR 30000000.0f
    _camera->setClippingPlane(SKYBOX_ZNEAR, SKYBOX_ZFAR);
    _camera->updateProjection();
    drawSpace(_camera->projectionMatrix() * _camera->viewMatrix());
    drawSun(0.0f, _camera->projectionMatrix() * _camera->viewMatrix());
}

void SpaceRenderStage::setState(vg::FrameBuffer* frameBuffer /*= nullptr*/) {
    throw std::logic_error("The method or operation is not implemented.");
}

bool SpaceRenderStage::isVisible() {
    throw std::logic_error("The method or operation is not implemented.");
}

void SpaceRenderStage::drawSpace(glm::mat4 &VP) {

    //f32m4 IMVP;

    //if (connectedToPlanet) {
    //    // If we are connected to the planet, we need to rotate the skybox
    //    IMVP = VP * GameManager::planet->invRotationMatrix;
    //} else {
    //    IMVP = VP;
    //}

    glDepthMask(GL_FALSE);
    _skyboxRenderer->drawSkybox(_glProgram, VP, skyboxTextures);
    drawSun((float)0, VP);
    glDepthMask(GL_TRUE);
}

// Ben: This is terrible but I don't feel like fixing it since its temporary
void SpaceRenderStage::drawSun(float theta, glm::mat4 &MVP) {
    double radius = 2800000.0;
    double size = 200000.0;
    float off = (float)atan(size / radius); // in radians
    float cosTheta = cos(theta - off);
    float sinTheta = sin(theta - off);
    float cosTheta2 = cos(theta + off);
    float sinTheta2 = sin(theta + off);

    // Bind shader
    _glProgram->use();
    _glProgram->enableVertexAttribArrays();

    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sunTexture.ID);
    // Set our "myTextureSampler" sampler to user Texture Unit 0
    glUniform1i(_glProgram->getUniform("myTextureSampler"), 0);

    glUniformMatrix4fv(_glProgram->getUniform("MVP"), 1, GL_FALSE, &MVP[0][0]);

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

    _glProgram->disableVertexAttribArrays();
    _glProgram->unuse();

    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
}
