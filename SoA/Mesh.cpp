#include "stdafx.h"
#include "Mesh.h"
#include "GLProgram.h"

#undef NDEBUG

namespace vorb{
namespace core{

#pragma region Default Shader

// Default shader source
const cString Mesh::defaultVertexShaderSource = R"(#version 130
uniform mat4 MVP;

in vec3 vPosition;
in vec4 vTint;
in vec2 vUV;

out vec2 fUV;
out vec4 fTint;

void main() {
    fTint = vTint;
    fUV = vUV;
    gl_Position = MVP * vec4(vPosition, 1.0);
}
)";
const cString Mesh::defaultFragmentShaderSource = R"(#version 130
uniform sampler2D tex;

in vec2 fUV;
in vec4 fTint;

out vec4 fColor;

void main() {
    fColor = texture(tex, fUV) * fTint;
}
)";

// Default shader attributes
const std::vector<std::pair<nString, ui32> > Mesh::defaultShaderAttributes = { 
    std::pair<nString, ui32>("vPosition", 0),
    std::pair<nString, ui32>("vTint", 1),
    std::pair<nString, ui32>("vUV", 2)
};

#pragma endregion

Mesh::Mesh() :
    _modelMatrix(1.0f),
    _vbo(0),
    _vao(0),
    _ibo(0),
    _isUploaded(0),
    _isIndexed(0),
    _numVertices(0),
    _numIndices(0),
    _primitiveType(PrimitiveType::TRIANGLES)
{
};

Mesh::~Mesh() {
    destroy();
}

void Mesh::destroy() {
    if (_vao != 0) {
        glDeleteVertexArrays(1, &_vao);
        _vao = 0;
    }
    if (_vbo != 0) {
        glDeleteBuffers(1, &_vbo);
        _vbo = 0;
    }
    if (_ibo != 0) {
        glDeleteBuffers(1, &_ibo);
        _ibo = 0;
    }
    std::vector <MeshVertex>().swap(_vertices);
    std::vector <ui32>().swap(_indices);
    _isUploaded = false;
}

void Mesh::init(PrimitiveType primitiveType, bool isIndexed) {
    _primitiveType = primitiveType;
    _isIndexed = isIndexed;
    createVertexArray();
}

void Mesh::reserve(int numVertices, int numIndices) {
    _vertices.reserve(numVertices);
    _indices.reserve(numIndices);
}

void Mesh::draw() {
    // Need to have uploaded our data
    assert(_isUploaded);
    // A shader needs to be bound
    assert(GLProgram::getCurrentProgram() != nullptr);

    // Bind the VAO
    glBindVertexArray(_vao);
    // Perform draw call
    if (_isIndexed) {
        glDrawElements(static_cast<GLenum>(_primitiveType), _numIndices, GL_UNSIGNED_INT, (void*)0);
    } else {
        glDrawArrays(static_cast<GLenum>(_primitiveType), 0, _numVertices);
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
    i = _indices.size();
    j = 0;
    _indices.resize(_indices.size() + newIndices.size());
    while (i < _indices.size()) {
        _indices[i++] = newIndices[j++];
    }
}

void Mesh::uploadAndClearLocal(MeshUsage usage) {
    upload(usage);
    std::vector <MeshVertex>().swap(_vertices);
    std::vector <ui32>().swap(_indices);
}

void Mesh::uploadAndKeepLocal(MeshUsage usage) {
    upload(usage);
}

void Mesh::upload(MeshUsage usage) {
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(MeshVertex)* _vertices.size(), nullptr, static_cast<GLenum>(usage));
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(MeshVertex)* _vertices.size(), _vertices.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    _numVertices = _vertices.size();

    if (_isIndexed) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ui32)* _indices.size(), nullptr, static_cast<GLenum>(usage));
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(ui32)* _indices.size(), _indices.data());
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    _numIndices = _indices.size();

    _isUploaded = true;
}

int Mesh::getNumPrimitives() const {
    // If indexed, we use indices. Otherwise verts
    int n;
    if (!_isUploaded) {
        if (_isIndexed) {
            n = _indices.size();
        } else {
            n = _vertices.size();
        }
    } else {
        if (_isIndexed) {
            n = _numIndices;
        } else {
            n = _numVertices;
        }
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
    // Generate and bind element buffer
    if (_isIndexed) {
        glGenBuffers(1, &_ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
    }
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