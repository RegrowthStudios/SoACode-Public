#pragma once
#include <chrono>

#include <Vorb/VorbPreDecl.inl>
#include <Vorb/graphics/GLProgram.h>

DECL_VG(class GLProgramManager);

const static float GOLDEN_RATIO = 1.61803398875f;

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

class SimpleMesh {
public:
    GLuint vertexBufferID;
    GLuint indexBufferID;
    int numVertices;
    int numIndices;
};

class Icosphere {
public:
    glm::vec3 position;
    float radius;
    glm::vec4 color;
    int lod;
    double timeTillDeletion; //ms
};

class Cube {
public:
    glm::vec3 position;
    glm::vec3 size;
    glm::vec4 color;
    double timeTillDeletion; //ms
};

class Line {
public:
    glm::vec3 position1;
    glm::vec3 position2;
    glm::vec4 color;
    double timeTillDeletion; //ms
};

class DebugRenderer {
public:
    DebugRenderer();
    ~DebugRenderer();

    void render(const glm::mat4 &vp, const glm::vec3& playerPos, const f32m4& w = f32m4(1.0));

    void drawIcosphere(const glm::vec3 &position, const float radius, const glm::vec4 &color, const int lod, const double duration = -1.0f);
    void drawCube(const glm::vec3 &position, const glm::vec3 &size, const glm::vec4 &color, const double duration = -1.0f);
    void drawLine(const glm::vec3 &startPoint, const glm::vec3 &endPoint, const glm::vec4 &color, const double duration = -1.0f);

private:
    void renderIcospheres(const glm::mat4 &vp, const f32m4& w, const glm::vec3& playerPos, const double deltaT);
    void renderCubes(const glm::mat4 &vp, const f32m4& w, const glm::vec3& playerPos, const double deltaT);
    void renderLines(const glm::mat4 &v, const f32m4& w, const glm::vec3& playerPosp, const double deltaT);

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
    vg::GLProgram* m_program = nullptr;

    static f32m4 _modelMatrix; ///< Reusable model matrix

    std::chrono::time_point<std::chrono::system_clock> _previousTimePoint;
    std::chrono::time_point<std::chrono::system_clock> _currentTimePoint;
};