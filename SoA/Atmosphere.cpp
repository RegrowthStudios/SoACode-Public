#include "stdafx.h"
#include "Atmosphere.h"

#include "ObjectLoader.h"
#include "GameManager.h"
#include "GLProgramManager.h"

#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\quaternion.hpp>
#include <glm\gtx\quaternion.hpp>
#include <glm\gtc\matrix_transform.hpp>

Atmosphere::Atmosphere() {
    m_Kr = 0.0025f;        // Rayleigh scattering constant
    m_Km = 0.0020f;        // Mie scattering constant
    m_ESun = 25.0f;        // Sun brightness constant
    m_g = -0.990f;        // The Mie phase asymmetry factor
    m_fExposure = 2.0f;
    m_fRayleighScaleDepth = 0.25f; //0.25

    m_fWavelength[0] = 0.650f;        // 650 nm for red
    m_fWavelength[1] = 0.570f;        // 570 nm for green
    m_fWavelength[2] = 0.475f;        // 475 nm for blue
    m_fWavelength4[0] = powf(m_fWavelength[0], 4.0f);
    m_fWavelength4[1] = powf(m_fWavelength[1], 4.0f);
    m_fWavelength4[2] = powf(m_fWavelength[2], 4.0f);

    nSamples = 3;
    fSamples = (float)nSamples;
}

void Atmosphere::initialize(nString filePath, float PlanetRadius) {
    if (filePath.empty()) {
        m_Kr = 0.0025;
        m_Km = 0.0020;
        m_ESun = 25.0;
        m_g = -0.990;
        m_fExposure = 2.0;
        m_fWavelength[0] = 0.650;
        m_fWavelength[1] = 0.570;
        m_fWavelength[2] = 0.475;
        nSamples = 3;

        fSamples = (float)nSamples;
        m_fWavelength4[0] = powf(m_fWavelength[0], 4.0f);
        m_fWavelength4[1] = powf(m_fWavelength[1], 4.0f);
        m_fWavelength4[2] = powf(m_fWavelength[2], 4.0f);
    } else {
        //    loadProperties(filePath);
    }

    radius = PlanetRadius*(1.025);
    planetRadius = PlanetRadius;

    std::cout << "Loading Objects/icosphere.obj... ";
    ObjectLoader objectLoader;
    objectLoader.load("Objects/icosphere.obj", vertices, indices);
    std::cout << "Done!\n";

    glGenBuffers(1, &(vboID)); // Create the buffer ID
    glBindBuffer(GL_ARRAY_BUFFER, vboID); // Bind the buffer (vertex array data)
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(ColorVertex), NULL, GL_STATIC_DRAW);

    glGenBuffers(1, &(vboIndexID));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndexID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), NULL, GL_STATIC_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(ColorVertex), &(vertices[0].position));
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(GLushort), &(indices[0]));
    indexSize = indices.size();

    vertices.clear();
    indices.clear();
}

void Atmosphere::draw(float theta, const glm::mat4 &MVP, glm::vec3 lightPos, const glm::dvec3 &ppos) {
    vg::GLProgram* shader = nullptr; // GameManager::glProgramManager->getProgram("Sky");
    shader->use();

    glm::mat4 GlobalModelMatrix(1.0);
    GlobalModelMatrix[0][0] = radius;
    GlobalModelMatrix[1][1] = radius;
    GlobalModelMatrix[2][2] = radius;
    GlobalModelMatrix[3][0] = (f32)-ppos.x;
    GlobalModelMatrix[3][1] = (f32)-ppos.y;
    GlobalModelMatrix[3][2] = (f32)-ppos.z;

    // Have to rotate it and draw it again to make a sphere
    f32v3 EulerAngles(M_PI, 0, 0);
    f32m4 RotationMatrix = glm::toMat4(glm::quat(EulerAngles));
    f32m4 MVPr = MVP * GlobalModelMatrix;
    f32m4 M = GlobalModelMatrix;

    f32 m_Kr4PI = m_Kr * 4.0f* M_PI;
    f32 m_Km4PI = m_Km * 4.0f* M_PI;
    f32 m_fScale = 1.0 / (radius - planetRadius);

    glUniformMatrix4fv(shader->getUniform("unWVP"), 1, GL_FALSE, &MVPr[0][0]);
    glUniform3f(shader->getUniform("unCameraPos"), (float)ppos.x, (float)ppos.y, (float)ppos.z);
    glUniform3f(shader->getUniform("unLightPos"), lightPos.x, lightPos.y, lightPos.z);
    glUniform3f(shader->getUniform("unInvWavelength"), 1 / m_fWavelength4[0], 1 / m_fWavelength4[1], 1 / m_fWavelength4[2]);
    glUniform1f(shader->getUniform("unCameraHeight2"), glm::length(ppos) * glm::length(ppos));
    glUniform1f(shader->getUniform("unOuterRadius"), radius);
    glUniform1f(shader->getUniform("unOuterRadius2"), radius * radius);
    glUniform1f(shader->getUniform("unInnerRadius"), planetRadius);
    glUniform1f(shader->getUniform("unKrESun"), m_Kr*m_ESun);
    glUniform1f(shader->getUniform("unKmESun"), m_Km*m_ESun);
    glUniform1f(shader->getUniform("unKr4PI"), m_Kr4PI);
    glUniform1f(shader->getUniform("unKm4PI"), m_Km4PI);
    glUniform1f(shader->getUniform("unScale"), m_fScale);
    glUniform1f(shader->getUniform("unScaleDepth"), m_fRayleighScaleDepth);
    glUniform1f(shader->getUniform("unScaleOverScaleDepth"), m_fScale / m_fRayleighScaleDepth);
    glUniform1f(shader->getUniform("unG"), m_g);
    glUniform1f(shader->getUniform("unG2"), m_g*m_g);
    glUniform1i(shader->getUniform("unNumSamples"), nSamples);
    glUniform1f(shader->getUniform("unNumSamplesF"), fSamples);

    glBindBuffer(GL_ARRAY_BUFFER, vboID);

    shader->enableVertexAttribArrays();

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ColorVertex), (void*)0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndexID);

    glDrawElements(GL_TRIANGLES, indexSize, GL_UNSIGNED_SHORT, 0);

    shader->disableVertexAttribArrays();

    shader->unuse();
}