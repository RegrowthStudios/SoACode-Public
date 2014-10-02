#include "stdafx.h"
#include "Mesh.h"
#include "GLProgram.h"

namespace vorb{
namespace core{

#pragma region Shader Source

const cString VS_SRC = R"(#version 130
uniform mat4 MVP;

in vec3 vPosition;
in vec2 vUV;
in vec4 vTint;

out vec2 fUV;
out vec4 fTint;

void main() {
    fTint = vTint;
    fUV = vUV;
    gl_Position = MVP * vec4(vPosition, 1.0);
}
)";
const cString FS_SRC = R"(#version 130
uniform sampler2D SBTex;

in vec2 fUV;
in vec4 fTint;

out vec4 fColor;

void main() {
    fColor = texture(SBTex, fUV) * fTint;
}
)";

#pragma endregion

Mesh::Mesh() :
    _modelMatrix(1.0f),
    _vbo(0),
    _vao(0),
    _primitiveType(PrimitiveType::TRIANGLES)
{
};

Mesh::~Mesh()
{
}

void Mesh::init(PrimitiveType primitiveType, bool isIndexed) {
    _primitiveType = primitiveType;
    _isIndexed = isIndexed;
    createVertexArray();
}

void Mesh::reserve(int numVertices) {
    _vertices.reserve(numVertices);
}

void Mesh::draw(const f32m4& viewProjectionMatrix, const SamplerState* ss, const DepthState* ds, const RasterizerState* rs) {
    // A shader needs to be bound
    assert(GLProgram::getCurrentProgram() != nullptr);
    // Set depth and sampler states
    ds->set();
    rs->set();
    // TODO(Ben): shader and stuff
    glBindVertexArray(_vao);
    // Perform draw call
    if (_isIndexed) {
        glDrawElements(static_cast <GLenum>(_primitiveType), 0, getNumPrimitives(), (void*)0);
    } else {
        glDrawArrays(static_cast<GLenum>(_primitiveType), 0, _vertices.size());
    }
    glBindVertexArray(0);
}

void Mesh::addVertices(const std::vector<MeshVertex>& newVertices) {
    // This should only be called when not indexed
    assert(!_isIndexed);
    // Add the newVertices onto the _vertices array
    int i = _vertices.size();
    int j = 0;
    _vertices.resize(_vertices.size() + newVertices.size());
    while (i < _vertices.size()) {
        _vertices[i++] = newVertices[j++];
    }
}

void Mesh::addVertices(const std::vector<MeshVertex>& newVertices, const std::vector<ui32>& newIndices) {
    // This should only be called when indexed
    assert(_isIndexed);
    // Add the newVertices onto the _vertices array
    int i = _vertices.size();
    int j = 0;
    _vertices.resize(_vertices.size() + newVertices.size());
    while (i < _vertices.size()) {
        _vertices[i++] = newVertices[j++];
    }
    // Add the newIndices onto the _indices array
    int i = _indices.size();
    int j = 0;
    _indices.resize(_indices.size() + newIndices.size());
    while (i < _indices.size()) {
        _indices[i++] = newIndices[j++];
    }
}

int Mesh::getNumPrimitives() const {
    // If indexed, we use indices. Otherwise verts
    int n;
    if (_isIndexed) {
        n = _indices.size();
    } else {
        n = _vertices.size();
    }
    // Primitive count is based on type
    switch (_primitiveType) {
        case PrimitiveType::LINES:
            return n / 2;
        case PrimitiveType::POINTS:
            return n;
        case PrimitiveType::TRIANGLES:
            return n / 3;
    }
    return 0;
}

void Mesh::createVertexArray() {
    // Generate and bind vao
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);
    // Generate and bind vbo
    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    // Set attribute arrays
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    // Set attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(MeshVertex), (void*)offsetof(MeshVertex, position));
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, true, sizeof(MeshVertex), (void*)offsetof(MeshVertex, color));
    glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(MeshVertex), (void*)offsetof(MeshVertex, uv));
    // Unbind vao
    glBindVertexArray(0);
}

}
}