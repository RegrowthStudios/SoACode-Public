#pragma once
#include <chrono>

#include <Vorb/VorbPreDecl.inl>
#include <Vorb/graphics/GLProgram.h>

const static float GOLDEN_RATIO = 1.61803398875f;

const static int NUM_CUBE_VERTICES = 8;
const static f32v3 CUBE_VERTICES[8] = {
    //front
    f32v3(-0.5f, -0.5f, 0.5f),
    f32v3(0.5f, -0.5f, 0.5f),
    f32v3(0.5f, 0.5f, 0.5f),
    f32v3(-0.5f, 0.5f, 0.5f),
    // back
    f32v3(-0.5f, -0.5f, -0.5f),
    f32v3(0.5f, -0.5f, -0.5f),
    f32v3(0.5f, 0.5f, -0.5f),
    f32v3(-0.5f, 0.5f, -0.5f),
};

const static int NUM_CUBE_INDICES = 36;
const static GLuint CUBE_INDICES[36] = {
    // front
    0, 1, 2,
    2, 3, 0,
    // top
    3, 2, 6,
    6, 7, 3,
    // back
    7, 6, 5,
    5, 4, 7,
    // bottom
    4, 5, 1,
    1, 0, 4,
    // left
    4, 0, 3,
    3, 7, 4,
    // right
    1, 5, 6,
    6, 2, 1,
};

class SimpleMesh {
public:
    GLuint vertexBufferID;
    GLuint indexBufferID;
    int numVertices;
    int numIndices;
};

class Icosphere {
public:
    f32v3 position;
    float radius;
    f32v4 color;
    int lod;
    double timeTillDeletion; //ms
};

class Cube {
public:
    f32v3 position;
    f32v3 size;
    f32v4 color;
    double timeTillDeletion; //ms
};

class Line {
public:
    f32v3 position1;
    f32v3 position2;
    f32v4 color;
    double timeTillDeletion; //ms
};

class DebugRenderer {
public:
    DebugRenderer();
    ~DebugRenderer();

    void render(const f32m4 &vp, const f32v3& playerPos, const f32m4& w = f32m4(1.0));

    void drawIcosphere(const f32v3 &position, const float radius, const f32v4 &color, const int lod, const double duration = -1.0f);
    void drawCube(const f32v3 &position, const f32v3 &size, const f32v4 &color, const double duration = -1.0f);
    void drawLine(const f32v3 &startPoint, const f32v3 &endPoint, const f32v4 &color, const double duration = -1.0f);

private:
    void renderIcospheres(const f32m4& vp, const f32m4& w, const f32v3& playerPos, const double deltaT);
    void renderCubes(const f32m4& vp, const f32m4& w, const f32v3& playerPos, const double deltaT);
    void renderLines(const f32m4& v, const f32m4& w, const f32v3& playerPosp, const double deltaT);

    void createIcosphere(const int lod);

    SimpleMesh* DebugRenderer::createMesh(const f32v3* vertices, const int numVertices, const GLuint* indices, const int numIndices);
    SimpleMesh* createMesh(const std::vector<f32v3>& vertices, const std::vector<GLuint>& indices);

    std::vector<Icosphere> _icospheresToRender;
    std::vector<Cube> _cubesToRender;
    std::vector<Line> _linesToRender;

    //Icosphere meshes sorted by level of detail
    std::map<int, SimpleMesh*> _icosphereMeshes;
    SimpleMesh* _cubeMesh;
    SimpleMesh* _lineMesh;

    // Program that is currently in use
    vg::GLProgram m_program;

    static f32m4 _modelMatrix; ///< Reusable model matrix

    std::chrono::time_point<std::chrono::system_clock> _previousTimePoint;
    std::chrono::time_point<std::chrono::system_clock> _currentTimePoint;
};