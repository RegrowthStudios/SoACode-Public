#pragma once
#include <chrono>

#include <Vorb/GLProgram.h>

const static float GOLDEN_RATIO = 1.61803398875f;

const static int NUM_ICOSOHEDRON_VERTICES = 12;
const static glm::vec3 ICOSOHEDRON_VERTICES[12] = {
    glm::vec3(-1.0f, GOLDEN_RATIO, 0.0f),
    glm::vec3(1.0f, GOLDEN_RATIO, 0.0f),
    glm::vec3(-1.0f, -GOLDEN_RATIO, 0.0f),
    glm::vec3(1.0f, -GOLDEN_RATIO, 0.0f),

    glm::vec3(0.0f, -1.0f, GOLDEN_RATIO),//4
    glm::vec3(0.0f, 1.0f, GOLDEN_RATIO),
    glm::vec3(0.0f, -1.0, -GOLDEN_RATIO),
    glm::vec3(0.0f, 1.0f, -GOLDEN_RATIO),

    glm::vec3(GOLDEN_RATIO, 0.0f, -1.0f),//8
    glm::vec3(GOLDEN_RATIO, 0.0f, 1.0f),
    glm::vec3(-GOLDEN_RATIO, 0.0f, -1.0f),
    glm::vec3(-GOLDEN_RATIO, 0.0, 1.0f)
};

const static int NUM_ICOSOHEDRON_INDICES = 60;
const static GLuint ICOSOHEDRON_INDICES[60] = {
    0, 11, 5,
    0, 5, 1,
    0, 1, 7,
    0, 7, 10,
    0, 10, 11,

    1, 5, 9,
    5, 11, 4,
    11, 10, 2,
    10, 7, 6,
    7, 1, 8,

    3, 9, 4,
    3, 4, 2,
    3, 2, 6,
    3, 6, 8,
    3, 8, 9,

    4, 9, 5,
    2, 4, 11,
    6, 2, 10,
    8, 6, 7,
    9, 8, 1
};

const static int NUM_CUBE_VERTICES = 8;
const static glm::vec3 CUBE_VERTICES[8] = {
    //front
    glm::vec3(-0.5f, -0.5f, 0.5f),
    glm::vec3(0.5f, -0.5f, 0.5f),
    glm::vec3(0.5f, 0.5f, 0.5f),
    glm::vec3(-0.5f, 0.5f, 0.5f),
    // back
    glm::vec3(-0.5f, -0.5f, -0.5f),
    glm::vec3(0.5f, -0.5f, -0.5f),
    glm::vec3(0.5f, 0.5f, -0.5f),
    glm::vec3(-0.5f, 0.5f, -0.5f),
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

struct SimpleMesh {
    GLuint vertexBufferID;
    GLuint indexBufferID;
    int numVertices;
    int numIndices;
};

struct Icosphere {
    glm::vec3 position;
    float radius;
    glm::vec4 color;
    int lod;
    double timeTillDeletion; //ms
};

struct Cube {
    glm::vec3 position;
    glm::vec3 size;
    glm::vec4 color;
    double timeTillDeletion; //ms
};

struct Line {
    glm::vec3 position1;
    glm::vec3 position2;
    glm::vec4 color;
    double timeTillDeletion; //ms
};

class DebugRenderer {
public:
    DebugRenderer();
    ~DebugRenderer();

    void render(const glm::mat4 &vp, const glm::vec3& playerPos);

    void drawIcosphere(const glm::vec3 &position, const float radius, const glm::vec4 &color, const int lod, const double duration = -1.0f);
    void drawCube(const glm::vec3 &position, const glm::vec3 &size, const glm::vec4 &color, const double duration = -1.0f);
    void drawLine(const glm::vec3 &startPoint, const glm::vec3 &endPoint, const glm::vec4 &color, const double duration = -1.0f);

private:
    void renderIcospheres(const glm::mat4 &vp, const glm::vec3& playerPos, const double deltaT);
    void renderCubes(const glm::mat4 &vp, const glm::vec3& playerPos, const double deltaT);
    void renderLines(const glm::mat4 &v, const glm::vec3& playerPosp, const double deltaT);

    void createIcosphere(const int lod);

    SimpleMesh* DebugRenderer::createMesh(const glm::vec3* vertices, const int numVertices, const GLuint* indices, const int numIndices);
    SimpleMesh* createMesh(const std::vector<glm::vec3>& vertices, const std::vector<GLuint>& indices);

    std::vector<Icosphere> _icospheresToRender;
    std::vector<Cube> _cubesToRender;
    std::vector<Line> _linesToRender;

    //Icosphere meshes sorted by level of detail
    std::map<int, SimpleMesh*> _icosphereMeshes;
    SimpleMesh* _cubeMesh;
    SimpleMesh* _lineMesh;

    // Program that is currently in use
    vg::GLProgram* _program;

    std::chrono::time_point<std::chrono::system_clock> _previousTimePoint;
    std::chrono::time_point<std::chrono::system_clock> _currentTimePoint;
};