#pragma once

// Which Faces To Cull
enum class CullFaceMode : GLenum {
    BACK = GL_BACK,
    FRONT = GL_FRONT,
    FRONT_AND_BACK = GL_FRONT_AND_BACK
};
// Vertex Winding For A Front-Facing Triangle
enum class FrontFaceDirection : GLenum {
    CCW = GL_CCW,
    CW = GL_CW
};

// Specify How Triangles Are Drawn (Specifically Which Are Culled)
class RasterizerState {
public:
    RasterizerState(bool use, CullFaceMode cullFaceMode, FrontFaceDirection frontFaceDirection);

    // Apply State In The Rendering Pipeline
    void set() const;

    bool useCulling;
    CullFaceMode cullMode;
    FrontFaceDirection faceOrientation;

    static const RasterizerState CULL_NONE;
    static const RasterizerState CULL_CLOCKWISE;
    static const RasterizerState CULL_COUNTER_CLOCKWISE;
};