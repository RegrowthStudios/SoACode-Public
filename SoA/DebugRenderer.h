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

struct SimpleMeshVertex {
    f32v3 position;
    color4 color;
};

struct SimpleMesh {
    std::vector<f32v3> vertices;
    std::vector<ui32> indices;
};

struct Icosphere {
    f64v3 position;
    float radius;
    color4 color;
    int lod;
    double timeTillDeletion; //ms
};

struct Cube {
    f64v3 position;
    f64v3 size;
    color4 color;
    double timeTillDeletion; //ms
};

struct Line {
    f64v3 position1;
    f64v3 position2;
    color4 color;
    double timeTillDeletion; //ms
};

struct DebugRenderContext {
    std::vector<Icosphere> icospheres;
    std::vector<Cube> cubes;
    std::vector<Line> lines;
};

class DebugRenderer {
public:
    ~DebugRenderer();

    void render(const f32m4 &vp, const f64v3& viewPosition, const f32m4& w = f32m4(1.0));

    void drawIcosphere(const f64v3 &position, const float radius, const color4 &color, const int lod, ui32 context = 0, const double duration = FLT_MAX);
    void drawCube(const f64v3 &position, const f64v3 &size, const color4 &color, ui32 context = 0, const double duration = FLT_MAX);
    void drawLine(const f64v3 &startPoint, const f64v3 &endPoint, const color4 &color, ui32 context = 0, const double duration = FLT_MAX);

private:
    void renderIcospheres(std::vector<Icosphere>& icospheres, const f32m4& vp, const f32m4& w, const f64v3& viewPosition, const double deltaT);
    void renderCubes(std::vector<Cube>& cubes, const f32m4& vp, const f32m4& w, const f64v3& viewPosition, const double deltaT);
    void renderLines(std::vector<Line>& lines, const f32m4& v, const f32m4& w, const f64v3& viewPosition, const double deltaT);

    void createIcosphere(const int lod);

    std::map<ui32, DebugRenderContext> m_contexts;

    //Icosphere meshes sorted by level of detail
    std::map<int, SimpleMesh> m_icosphereMeshes;

    // Program that is currently in use
    vg::GLProgram m_program;

    VGBuffer m_vbo = 0;
    VGIndexBuffer m_ibo = 0;

    std::chrono::high_resolution_clock::time_point m_previousTimePoint;
    std::chrono::high_resolution_clock::time_point m_currentTimePoint;
};