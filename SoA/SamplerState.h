#pragma once

#ifndef GL_UNSIGNED_SHORT_5_6_5
#define GL_UNSIGNED_SHORT_5_6_5 0x8363
#endif
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

enum class TextureMinFilter : GLenum {
    LINEAR = GL_LINEAR,
    LINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR,
    LINEAR_MIPMAP_NEAREST = GL_LINEAR_MIPMAP_NEAREST,
    NEAREST = GL_NEAREST,
    NEAREST_MIPMAP_LINEAR = GL_NEAREST_MIPMAP_LINEAR,
    NEAREST_MIPMAP_NEAREST = GL_NEAREST_MIPMAP_NEAREST
};
enum class TextureMagFilter : GLenum {
    LINEAR = GL_LINEAR,
    NEAREST = GL_NEAREST
};
enum class TextureWrapMode : GLenum {
    CLAMP_BORDER = GL_CLAMP_TO_BORDER,
    CLAMP_EDGE = GL_CLAMP_TO_EDGE,
    REPEAT_MIRRORED = GL_MIRRORED_REPEAT,
    REPEAT = GL_REPEAT
};

class SamplerState {
public:
    SamplerState(TextureMinFilter texMinFilter, TextureMagFilter texMagFilter,
        TextureWrapMode texWrapS, TextureWrapMode texWrapT, TextureWrapMode texWrapR);
    SamplerState(ui32 texMinFilter, ui32 texMagFilter,
        ui32 texWrapS, ui32 texWrapT, ui32 texWrapR) :
        SamplerState(static_cast<TextureMinFilter>(texMinFilter), static_cast<TextureMagFilter>(texMagFilter),
        static_cast<TextureWrapMode>(texWrapS), static_cast<TextureWrapMode>(texWrapT), static_cast<TextureWrapMode>(texWrapR)) {}

    void initObject();
    // Initialize All The Sampler Objects When OpenGL Context Is Created
    static void initPredefined();

    // Target Is Of The Family GL_TEXTURE_2D/3D/etc. For Use On A Texture
    void set(ui32 textureTarget) const;
    // Unit Is In The Range [0 - GraphicsDeviceProperties::maxTextureUnits)
    void setObject(ui32 textureUnit) const;

    static SamplerState POINT_WRAP;
    static SamplerState POINT_CLAMP;
    static SamplerState LINEAR_WRAP;
    static SamplerState LINEAR_CLAMP;
    static SamplerState POINT_WRAP_MIPMAP;
    static SamplerState POINT_CLAMP_MIPMAP;
    static SamplerState LINEAR_WRAP_MIPMAP;
    static SamplerState LINEAR_CLAMP_MIPMAP;
private:
    ui32 _id;

    TextureMinFilter _minFilter;
    TextureMagFilter _magFilter;
    TextureWrapMode _wrapS;
    TextureWrapMode _wrapT;
    TextureWrapMode _wrapR;
};