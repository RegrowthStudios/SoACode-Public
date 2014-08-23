#pragma once

// Available Depth Comparisons
enum class DepthFunction : GLenum {
    NEVER = GL_NEVER,
    ALWAYS = GL_ALWAYS,
    EQUALS = GL_EQUAL,
    NOT_EQUAL = GL_NOTEQUAL,
    LESS_THAN = GL_LESS,
    LESS_EQUAL = GL_LEQUAL,
    GREATER_THAN = GL_GREATER,
    GREATER_EQUAL = GL_GEQUAL
};

// Encapsulates Z-Buffer Operations
class DepthState {
public:
    DepthState(bool read, DepthFunction depthFunction, bool write);
    DepthState(bool read, GLenum depthFunction, bool write) :
        DepthState(read, static_cast<DepthFunction>(depthFunction), write) {}

    // Apply State In The Rendering Pipeline
    void set() const;

    // Z-Buffer Access
    bool shouldRead;
    bool shouldWrite;

    // Comparison Against Destination Depth For Pixel Write/Discard
    DepthFunction depthFunc;

    // Always Draw Without Touching Depth
    static const DepthState NONE;
    // Only Draw When Depth Is Less Than Z-Buffer Without Modifying The Z-Buffer
    static const DepthState READ;
    // Always Draw And Overwrite The Z-Buffer With New Depth
    static const DepthState WRITE;
    // Only Draw When Depth Is Less Than Z-Buffer And Overwrite The Z-Buffer With New Depth
    static const DepthState FULL;
};