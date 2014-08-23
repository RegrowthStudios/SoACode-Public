#include "stdafx.h"
#include "GLTexture.h"

GLTexture::GLTexture(ui32 textureTarget, bool shouldInit /*= false*/) :
_id(0),
_textureTarget(textureTarget),
_internalFormat(GL_RGBA) {
    _texBinding = &_bindingMap.find(_textureTarget)->second;
    if (shouldInit) init();
}

void GLTexture::init() {
    if (getIsCreated()) return;
    glGenTextures(1, &_id);
}
void GLTexture::dispose() {
    if (getIsCreated()) {
        glDeleteTextures(1, &_id);
        _id = 0;
    }
}

void GLTexture::bind() {
    if (*_texBinding != this) {
        *_texBinding = this;
        glBindTexture(_textureTarget, _id);
    }
}
void GLTexture::unbind() {
    if (*_texBinding == this) {
        *_texBinding = nullptr;
        glBindTexture(_textureTarget, 0);
    }
}
void GLTexture::unbind(ui32 textureTarget) {
    _bindingMap[textureTarget]->unbind();
}

void GLTexture::bindToUnit(ui32 unit) {
    glActiveTexture(unit + GL_TEXTURE0);
    bind();
}
void GLTexture::setUniformSampler(ui32 textureUnit, ui32 uniformSampler) {
    glUniform1i(uniformSampler, textureUnit);
}

void GLTexture::use(ui32 textureUnit, ui32 uniformSampler) {
    bindToUnit(textureUnit);
    setUniformSampler(textureUnit, uniformSampler);
}
void GLTexture::unuse() {
    unbind();
}

std::map<ui32, GLTexture*> generateInitialTextureMap() {
    std::map<ui32, GLTexture*> map;
    map[GL_PROXY_TEXTURE_1D] = nullptr;
    map[GL_PROXY_TEXTURE_1D_ARRAY] = nullptr;
    map[GL_PROXY_TEXTURE_2D] = nullptr;
    map[GL_PROXY_TEXTURE_2D_ARRAY] = nullptr;
    map[GL_PROXY_TEXTURE_2D_MULTISAMPLE] = nullptr;
    map[GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY] = nullptr;
    map[GL_PROXY_TEXTURE_3D] = nullptr;
    map[GL_PROXY_TEXTURE_CUBE_MAP] = nullptr;
    map[GL_PROXY_TEXTURE_CUBE_MAP_ARRAY] = nullptr;
    map[GL_PROXY_TEXTURE_RECTANGLE] = nullptr;
    map[GL_TEXTURE_1D] = nullptr;
    map[GL_TEXTURE_1D_ARRAY] = nullptr;
    map[GL_TEXTURE_2D] = nullptr;
    map[GL_TEXTURE_2D_ARRAY] = nullptr;
    map[GL_TEXTURE_2D_MULTISAMPLE] = nullptr;
    map[GL_TEXTURE_2D_MULTISAMPLE_ARRAY] = nullptr;
    map[GL_TEXTURE_3D] = nullptr;
    map[GL_TEXTURE_BASE_LEVEL] = nullptr;
    map[GL_TEXTURE_BINDING_CUBE_MAP] = nullptr;
    map[GL_TEXTURE_BUFFER] = nullptr;
    map[GL_TEXTURE_CUBE_MAP] = nullptr;
    map[GL_TEXTURE_CUBE_MAP_ARRAY] = nullptr;
    map[GL_TEXTURE_CUBE_MAP_NEGATIVE_X] = nullptr;
    map[GL_TEXTURE_CUBE_MAP_NEGATIVE_Y] = nullptr;
    map[GL_TEXTURE_CUBE_MAP_NEGATIVE_Z] = nullptr;
    map[GL_TEXTURE_CUBE_MAP_POSITIVE_X] = nullptr;
    map[GL_TEXTURE_CUBE_MAP_POSITIVE_Y] = nullptr;
    map[GL_TEXTURE_CUBE_MAP_POSITIVE_Z] = nullptr;
    map[GL_TEXTURE_MAX_LEVEL] = nullptr;
    map[GL_TEXTURE_MAX_LOD] = nullptr;
    map[GL_TEXTURE_MIN_LOD] = nullptr;
    map[GL_TEXTURE_RECTANGLE] = nullptr;
    return map;
}
std::map<ui32, GLTexture*> GLTexture::_bindingMap = generateInitialTextureMap();