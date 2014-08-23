#pragma once

#define DIMENSION_WIDTH_INDEX 0
#define DIMENSION_HEIGHT_INDEX 1
#define DIMENSION_DEPTH_INDEX 2

class GLTexture {
public:
    GLTexture(ui32 textureTarget, bool shouldInit = false);

    const ui32 getID() const {
        return _id;
    }
    const ui32 getTextureTarget() const {
        return _textureTarget;
    }
    const ui32 getInternalPixelFormat() const {
        return _internalFormat;
    }
    void setInternalPixelFormat(ui32 format) {
        _internalFormat = format;
    }

    ui32 getWidth() const {
        return _dimensions[0];
    }
    ui32 getHeight() const {
        return _dimensions[1];
    }
    ui32 getDepth() const {
        return _dimensions[2];
    }
    ui32 getDimension(ui32 axis) const {
        return _dimensions[axis];
    }

    bool getIsCreated() const {
        return _id != 0;
    }

    void init();
    void dispose();

    void bind();
    void unbind();
    static void unbind(ui32 textureTarget);

    void bindToUnit(ui32 unit);
    void setUniformSampler(ui32 textureUnit, ui32 uniformSampler);

    void use(ui32 textureUnit, ui32 uniformSampler);
    void unuse();
private:
    ui32 _id;

    ui32 _dimensions[3];

    ui32 _textureTarget;
    GLTexture** _texBinding;

    ui32 _internalFormat;

    static std::map<ui32, GLTexture*> _bindingMap;
};