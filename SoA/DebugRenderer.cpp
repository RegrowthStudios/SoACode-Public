#include "stdafx.h"
#include "DebugRenderer.h"

#include <algorithm>
#include <functional>

#include <Vorb/MeshGenerators.h>
#include <Vorb/graphics/RasterizerState.h>

#include "GameManager.h"
#include "GLProgramManager.h"
#include "global.h"
#include "RenderUtils.h"

f32m4 DebugRenderer::_modelMatrix(1.0);

glm::vec3 findMidpoint(glm::vec3 vertex1, glm::vec3 vertex2);

class Vec3KeyFuncs {
public:
    size_t operator()(const glm::vec3& k)const {
        return std::hash<float>()(k.x) ^ std::hash<float>()(k.y) ^ std::hash<float>()(k.z);
    }

    bool operator()(const glm::vec3& a, const glm::vec3& b)const {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    }
};

DebugRenderer::DebugRenderer(const vg::GLProgramManager* glProgramManager) :
    _cubeMesh(nullptr),
    _lineMesh(nullptr),
    m_glProgramManager(glProgramManager)
{}

DebugRenderer::~DebugRenderer() {
    for(auto i = _icosphereMeshes.begin(); i != _icosphereMeshes.end(); i++) {
        glDeleteBuffers(1, &i->second->indexBufferID);
        glDeleteBuffers(1, &i->second->vertexBufferID);
        delete i->second;
    }
    if(_cubeMesh) {
        glDeleteBuffers(1, &_cubeMesh->indexBufferID);
        glDeleteBuffers(1, &_cubeMesh->vertexBufferID);
        delete _cubeMesh;
    }
    if(_lineMesh) {
        glDeleteBuffers(1, &_lineMesh->indexBufferID);
        glDeleteBuffers(1, &_lineMesh->vertexBufferID);
        delete _lineMesh;
    }
}

void DebugRenderer::render(const glm::mat4 &vp, const glm::vec3& playerPos, const f32m4& w /* = f32m4(1.0) */) {
    RasterizerState::CULL_NONE.set();

    _previousTimePoint = _currentTimePoint;
    _currentTimePoint = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedTime = _currentTimePoint - _previousTimePoint;
    double deltaT = elapsedTime.count();

    _program = m_glProgramManager->getProgram("BasicColor");

    _program->use();
    _program->enableVertexAttribArrays();
    
    if(_icospheresToRender.size() > 0) renderIcospheres(vp, w, playerPos, deltaT);
    if(_cubesToRender.size() > 0) renderCubes(vp, w, playerPos, deltaT);
    if(_linesToRender.size() > 0) renderLines(vp, w, playerPos, deltaT);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    _program->disableVertexAttribArrays();
    _program->use();
    
}

void DebugRenderer::drawIcosphere(const glm::vec3 &position, const float radius, const glm::vec4 &color, const int lod, const double duration) {
    auto lookup = _icosphereMeshes.find(lod);
    if(lookup == _icosphereMeshes.end()) {
        createIcosphere(lod);
    }
    Icosphere sphere;
    sphere.color = glm::vec4(color);
    sphere.lod = lod;
    sphere.position = glm::vec3(position);
    sphere.radius = radius;
    sphere.timeTillDeletion = duration;
    _icospheresToRender.push_back(sphere);
}

void DebugRenderer::drawCube(const glm::vec3 &position, const glm::vec3 &size, const glm::vec4 &color, const double duration) {
    if(!_cubeMesh) {
        _cubeMesh = createMesh(CUBE_VERTICES, NUM_CUBE_VERTICES, CUBE_INDICES, NUM_CUBE_INDICES);
    }
    Cube cube;
    cube.position = glm::vec3(position);
    cube.size = glm::vec3(size);
    cube.color = glm::vec4(color);
    cube.timeTillDeletion = duration;
    _cubesToRender.push_back(cube);
}

void DebugRenderer::drawLine(const glm::vec3 &startPoint, const glm::vec3 &endPoint, const glm::vec4 &color, const double duration) {
    if(!_lineMesh) {
        glm::vec3 vertices[2] { glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f) };
        GLuint indices[2] { 0, 1 };
        _lineMesh = createMesh(vertices, 2, indices, 2);
    }
    Line line;
    line.color = glm::vec4(color);
    line.position1 = glm::vec3(startPoint);
    line.position2 = glm::vec3(endPoint);
    line.timeTillDeletion = duration;
    _linesToRender.push_back(line);
}

void DebugRenderer::renderIcospheres(const glm::mat4 &vp, const f32m4& w, const glm::vec3& playerPos, const double deltaT) {
    for(auto i = _icospheresToRender.begin(); i != _icospheresToRender.end(); i++) {
        SimpleMesh* mesh = _icosphereMeshes.at(i->lod);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBufferID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBufferID);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

        setMatrixTranslation(_modelMatrix, i->position, playerPos);
        setMatrixScale(_modelMatrix, i->radius, i->radius, i->radius);
        glm::mat4 mvp = vp * _modelMatrix * w;

        glUniform4f(_program->getUniform("unColor"), i->color.r, i->color.g, i->color.b, i->color.a);
        glUniformMatrix4fv(_program->getUniform("unWVP"), 1, GL_FALSE, &mvp[0][0]);
        glDrawElements(GL_TRIANGLES, mesh->numIndices, GL_UNSIGNED_INT, 0);

        i->timeTillDeletion -= deltaT;
    }

    _icospheresToRender.erase(std::remove_if(_icospheresToRender.begin(), _icospheresToRender.end(), [](const Icosphere& sphere) { return sphere.timeTillDeletion <= 0; }), _icospheresToRender.end());
}

void DebugRenderer::renderCubes(const glm::mat4 &vp, const f32m4& w, const glm::vec3& playerPos, const double deltaT) {
    glBindBuffer(GL_ARRAY_BUFFER, _cubeMesh->vertexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _cubeMesh->indexBufferID);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
    for(auto i = _cubesToRender.begin(); i != _cubesToRender.end(); i++) {
        setMatrixTranslation(_modelMatrix, i->position, playerPos);
        setMatrixScale(_modelMatrix, i->size);
        glm::mat4 mvp = vp * _modelMatrix * w;

        glUniform4f(_program->getUniform("unColor"), i->color.r, i->color.g, i->color.b, i->color.a);
        glUniformMatrix4fv(_program->getUniform("unWVP"), 1, GL_FALSE, &mvp[0][0]);

        glDrawElements(GL_TRIANGLES, _cubeMesh->numIndices, GL_UNSIGNED_INT, 0);

        i->timeTillDeletion -= deltaT;
    }
    _cubesToRender.erase(std::remove_if(_cubesToRender.begin(), _cubesToRender.end(), [](const Cube& cube) { return cube.timeTillDeletion <= 0; }), _cubesToRender.end());
}

void DebugRenderer::renderLines(const glm::mat4 &vp, const f32m4& w, const glm::vec3& playerPos, const double deltaT) {
    glBindBuffer(GL_ARRAY_BUFFER, _lineMesh->vertexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _lineMesh->indexBufferID);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
    setMatrixScale(_modelMatrix, 1.0f, 1.0f, 1.0f);
    for(auto i = _linesToRender.begin(); i != _linesToRender.end(); i++) {
        glUniform4f(_program->getUniform("unColor"), i->color.r, i->color.g, i->color.b, i->color.a);
        setMatrixTranslation(_modelMatrix, i->position1, playerPos);
        
        glm::mat4 mvp = vp * _modelMatrix * w;
        glUniformMatrix4fv(_program->getUniform("unWVP"), 1, GL_FALSE, &mvp[0][0]);
        glm::vec3 secondVertex = i->position2 - i->position1;
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(glm::vec3), sizeof(glm::vec3), &secondVertex);
        glDrawElements(GL_LINES, _lineMesh->numIndices, GL_UNSIGNED_INT, 0);
        i->timeTillDeletion -= deltaT;
    }
    _linesToRender.erase(std::remove_if(_linesToRender.begin(), _linesToRender.end(), [](const Line& line) { return line.timeTillDeletion <= 0; }), _linesToRender.end());
}

void DebugRenderer::createIcosphere(const int lod) {
    std::vector<ui32> indices;
    std::vector<f32v3> positions;
    vmesh::generateIcosphereMesh(lod, indices, positions);
    _icosphereMeshes[lod] = createMesh(positions.data(), positions.size(), indices.data(), indices.size());
}

inline glm::vec3 findMidpoint(glm::vec3 vertex1, glm::vec3 vertex2) {
    return glm::normalize(glm::vec3((vertex1.x + vertex2.x) / 2.0f, (vertex1.y + vertex2.y) / 2.0f, (vertex1.z + vertex2.z) / 2.0f));
}

SimpleMesh* DebugRenderer::createMesh(const glm::vec3* vertices, const int numVertices, const GLuint* indices, const int numIndices) {
    SimpleMesh* mesh = new SimpleMesh();

    mesh->numVertices = numVertices;
    mesh->numIndices = numIndices;

    glGenBuffers(1, &mesh->vertexBufferID);
    glGenBuffers(1, &mesh->indexBufferID);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, mesh->numVertices * sizeof(glm::vec3), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->numIndices * sizeof(GLuint), indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return mesh;
}

SimpleMesh* DebugRenderer::createMesh(const std::vector<glm::vec3>& vertices, const std::vector<GLuint>& indices) {
    return createMesh(vertices.data(), vertices.size(), indices.data(), indices.size());
}