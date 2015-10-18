#include "stdafx.h"
#include "DebugRenderer.h"

#include <algorithm>
#include <functional>

#include <Vorb/MeshGenerators.h>
#include <Vorb/graphics/RasterizerState.h>
#include <Vorb/graphics/ShaderManager.h>

#include "GameManager.h"
#include "RenderUtils.h"
#include "ShaderLoader.h"

namespace {
    const cString VERT_SRC = R"(
// Uniforms
uniform mat4 unWVP;

// Input
in vec4 vPosition; // Position in object space
in vec4 vColor;

out vec4 fColor;

void main() {
  fColor = vColor;
  gl_Position = unWVP * vPosition;
}
)";
    const cString FRAG_SRC = R"(
in vec4 fColor;

// Output
out vec4 pColor;

void main() {
  pColor = fColor;
}

)";
}

f32v3 findMidpoint(f32v3 vertex1, f32v3 vertex2);

class Vec3KeyFuncs {
public:
    size_t operator()(const f32v3& k)const {
        return std::hash<float>()(k.x) ^ std::hash<float>()(k.y) ^ std::hash<float>()(k.z);
    }

    bool operator()(const f32v3& a, const f32v3& b)const {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    }
};

DebugRenderer::~DebugRenderer() {
   
    if (m_program.isCreated()) {
        m_program.dispose();
        glDeleteBuffers(1, &m_vbo);
        glDeleteBuffers(1, &m_ibo);
    }
}

void DebugRenderer::render(const f32m4 &vp, const f64v3& playerPos, const f32m4& w /* = f32m4(1.0) */) {

    vg::RasterizerState::CULL_NONE.set();

    m_previousTimePoint = m_currentTimePoint;
    m_currentTimePoint = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedTime = m_currentTimePoint - m_previousTimePoint;
    double deltaT = elapsedTime.count();

    if (!m_program.isCreated()) {
        m_program = ShaderLoader::createProgram("DebugRenderer", VERT_SRC, FRAG_SRC);
        glGenBuffers(1, &m_vbo);
        glGenBuffers(1, &m_ibo);
    }

    m_program.use();
    m_program.enableVertexAttribArrays();
    
    for (auto& c : m_contexts) {
     //   if (c.second.icospheres.size()) renderIcospheres(c.second.icospheres, vp, w, playerPos, deltaT);
     //   if (c.second.cubes.size()) renderCubes(c.second.cubes, vp, w, playerPos, deltaT);
        if (c.second.lines.size()) renderLines(c.second.lines, vp, w, playerPos, deltaT);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    m_program.disableVertexAttribArrays();
    m_program.use();
    
}

void DebugRenderer::drawIcosphere(const f64v3 &position, const float radius, const color4 &color, const int lod, ui32 context /*= 0*/, const double duration) {
    auto lookup = m_icosphereMeshes.find(lod);
    if(lookup == m_icosphereMeshes.end()) {
        createIcosphere(lod);
    }
    Icosphere sphere;
    sphere.color = color;
    sphere.lod = lod;
    sphere.position = position;
    sphere.radius = radius;
    sphere.timeTillDeletion = duration;
    m_contexts[context].icospheres.push_back(sphere);
}

void DebugRenderer::drawCube(const f64v3 &position, const f64v3 &size, const color4 &color, ui32 context /*= 0*/, const double duration) {

    Cube cube;
    cube.position = position;
    cube.size = size;
    cube.color = color;
    cube.timeTillDeletion = duration;
    m_contexts[context].cubes.push_back(cube);
}

void DebugRenderer::drawLine(const f64v3 &startPoint, const f64v3 &endPoint, const color4 &color, ui32 context /*= 0*/, const double duration) {
    Line line;
    line.color = color;
    line.position1 = startPoint;
    line.position2 = endPoint;
    line.timeTillDeletion = duration;
    m_contexts[context].lines.push_back(line);
}

void DebugRenderer::renderIcospheres(std::vector<Icosphere>& icospheres, const f32m4 &vp, const f32m4& w, const f64v3& playerPos, const double deltaT) {
   /* f32m4 modelMatrix(1.0f);
    for (auto i = icospheres.begin(); i != icospheres.end(); i++) {
        SimpleMesh* mesh = m_icosphereMeshes.at(i->lod);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBufferID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBufferID);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(f32v3), 0);

        setMatrixTranslation(modelMatrix, i->position, playerPos);
        setMatrixScale(modelMatrix, i->radius);
        f32m4 mvp = vp * modelMatrix * w;

        glUniform4f(m_program.getUniform("unColor"), i->color.r, i->color.g, i->color.b, i->color.a);
        glUniformMatrix4fv(m_program.getUniform("unWVP"), 1, GL_FALSE, &mvp[0][0]);
        glDrawElements(GL_TRIANGLES, mesh->numIndices, GL_UNSIGNED_INT, 0);

        i->timeTillDeletion -= deltaT;
    }

    icospheres.erase(std::remove_if(icospheres.begin(), icospheres.end(), [](const Icosphere& sphere) { return sphere.timeTillDeletion <= 0; }), icospheres.end());*/
}

void DebugRenderer::renderCubes(std::vector<Cube>& cubes, const f32m4 &vp, const f32m4& w, const f64v3& playerPos, const double deltaT) {
  /*  glBindBuffer(GL_ARRAY_BUFFER, m_cubeMesh->vertexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_cubeMesh->indexBufferID);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(f32v3), 0);
    f32m4 modelMatrix(1.0f);
    for (auto i = cubes.begin(); i != cubes.end(); i++) {
        setMatrixTranslation(modelMatrix, i->position, playerPos);
        setMatrixScale(modelMatrix, i->size);
        f32m4 mvp = vp * modelMatrix * w;

        glUniform4f(m_program.getUniform("unColor"), i->color.r, i->color.g, i->color.b, i->color.a);
        glUniformMatrix4fv(m_program.getUniform("unWVP"), 1, GL_FALSE, &mvp[0][0]);

        glDrawElements(GL_TRIANGLES, m_cubeMesh->numIndices, GL_UNSIGNED_INT, 0);

        i->timeTillDeletion -= deltaT;
    }
    cubes.erase(std::remove_if(_cubesToRender.begin(), _cubesToRender.end(), [](const Cube& cube) { return cube.timeTillDeletion <= 0; }), cubes.end());*/
}

void DebugRenderer::renderLines(std::vector<Line>& lines, const f32m4 &vp, const f32m4& w, const f64v3& playerPos, const double deltaT) {
    std::vector<SimpleMeshVertex> m_vertices(lines.size() * 2);
    
    int index = 0;
    for (auto& l : lines) {
        m_vertices[index].position = f32v3(l.position1 - playerPos);
        m_vertices[index].color = l.color;
        m_vertices[index + 1].position = f32v3(l.position2 - playerPos);
        m_vertices[index + 1].color = l.color;
        index += 2;
        l.timeTillDeletion -= deltaT;
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(SimpleMeshVertex), nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_vertices.size() * sizeof(SimpleMeshVertex), m_vertices.data());
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glVertexAttribPointer(m_program.getAttribute("vPosition"), 3, GL_FLOAT, GL_FALSE, sizeof(SimpleMeshVertex), offsetptr(SimpleMeshVertex, position));
    glVertexAttribPointer(m_program.getAttribute("vColor"), 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(SimpleMeshVertex), offsetptr(SimpleMeshVertex, color));
    f32m4 mvp = vp * w;
    glUniformMatrix4fv(m_program.getUniform("unWVP"), 1, GL_FALSE, &mvp[0][0]);

    glDrawArrays(GL_LINES, 0, m_vertices.size());

    lines.erase(std::remove_if(lines.begin(), lines.end(), [](const Line& line) { return line.timeTillDeletion <= 0; }), lines.end());
}

void DebugRenderer::createIcosphere(const int lod) {
    std::vector<ui32> indices;
    std::vector<f32v3> positions;
    vmesh::generateIcosphereMesh(lod, indices, positions);
  //  m_icosphereMeshes[lod] = createMesh(positions.data(), positions.size(), indices.data(), indices.size());
}

inline f32v3 findMidpoint(const f32v3& vertex1, const f32v3& vertex2) {
    return vmath::normalize(f32v3((vertex1.x + vertex2.x) / 2.0f, (vertex1.y + vertex2.y) / 2.0f, (vertex1.z + vertex2.z) / 2.0f));
}
