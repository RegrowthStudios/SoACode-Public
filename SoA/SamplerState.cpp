#include "stdafx.h"
#include "SamplerState.h"

SamplerState::SamplerState(TextureMinFilter texMinFilter, TextureMagFilter texMagFilter,
    TextureWrapMode texWrapS, TextureWrapMode texWrapT, TextureWrapMode texWrapR) :
    _minFilter(texMinFilter),
    _magFilter(texMagFilter),
    _wrapS(texWrapS),
    _wrapT(texWrapT),
    _wrapR(texWrapR) {}

void SamplerState::initObject() {
    glGenSamplers(1, &_id);
    glSamplerParameteri(_id, GL_TEXTURE_MIN_FILTER, static_cast<GLenum>(_minFilter));
    glSamplerParameteri(_id, GL_TEXTURE_MAG_FILTER, static_cast<GLenum>(_magFilter));
    glSamplerParameteri(_id, GL_TEXTURE_WRAP_S, static_cast<GLenum>(_wrapS));
    glSamplerParameteri(_id, GL_TEXTURE_WRAP_T, static_cast<GLenum>(_wrapT));
    glSamplerParameteri(_id, GL_TEXTURE_WRAP_R, static_cast<GLenum>(_wrapR));
}
void SamplerState::initPredefined() {
    SamplerState::POINT_WRAP.initObject();
    SamplerState::POINT_CLAMP.initObject();
    SamplerState::LINEAR_WRAP.initObject();
    SamplerState::LINEAR_CLAMP.initObject();
    SamplerState::POINT_WRAP_MIPMAP.initObject();
    SamplerState::POINT_CLAMP_MIPMAP.initObject();
    SamplerState::LINEAR_WRAP_MIPMAP.initObject();
    SamplerState::LINEAR_CLAMP_MIPMAP.initObject();
}

void SamplerState::set(ui32 textureTarget) const {
    glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, static_cast<GLenum>(_magFilter));
    glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, static_cast<GLenum>(_minFilter));
    glTexParameteri(textureTarget, GL_TEXTURE_WRAP_S, static_cast<GLenum>(_wrapS));
    glTexParameteri(textureTarget, GL_TEXTURE_WRAP_T, static_cast<GLenum>(_wrapT));
    glTexParameteri(textureTarget, GL_TEXTURE_WRAP_R, static_cast<GLenum>(_wrapR));
}
void SamplerState::setObject(ui32 textureUnit) const {
    glBindSampler(textureUnit, _id);
}

SamplerState SamplerState::POINT_WRAP(TextureMinFilter::NEAREST, TextureMagFilter::NEAREST,
    TextureWrapMode::REPEAT, TextureWrapMode::REPEAT, TextureWrapMode::REPEAT);
SamplerState SamplerState::POINT_CLAMP(TextureMinFilter::NEAREST, TextureMagFilter::NEAREST,
    TextureWrapMode::CLAMP_EDGE, TextureWrapMode::CLAMP_EDGE, TextureWrapMode::CLAMP_EDGE);
SamplerState SamplerState::LINEAR_WRAP(TextureMinFilter::LINEAR, TextureMagFilter::LINEAR,
    TextureWrapMode::REPEAT, TextureWrapMode::REPEAT, TextureWrapMode::REPEAT);
SamplerState SamplerState::LINEAR_CLAMP(TextureMinFilter::LINEAR, TextureMagFilter::LINEAR,
    TextureWrapMode::CLAMP_EDGE, TextureWrapMode::CLAMP_EDGE, TextureWrapMode::CLAMP_EDGE);
SamplerState SamplerState::POINT_WRAP_MIPMAP(TextureMinFilter::NEAREST_MIPMAP_NEAREST, TextureMagFilter::NEAREST,
    TextureWrapMode::REPEAT, TextureWrapMode::REPEAT, TextureWrapMode::REPEAT);
SamplerState SamplerState::POINT_CLAMP_MIPMAP(TextureMinFilter::NEAREST_MIPMAP_NEAREST, TextureMagFilter::NEAREST,
    TextureWrapMode::CLAMP_EDGE, TextureWrapMode::CLAMP_EDGE, TextureWrapMode::CLAMP_EDGE);
SamplerState SamplerState::LINEAR_WRAP_MIPMAP(TextureMinFilter::LINEAR_MIPMAP_LINEAR, TextureMagFilter::LINEAR,
    TextureWrapMode::REPEAT, TextureWrapMode::REPEAT, TextureWrapMode::REPEAT);
SamplerState SamplerState::LINEAR_CLAMP_MIPMAP(TextureMinFilter::LINEAR_MIPMAP_LINEAR, TextureMagFilter::LINEAR,
    TextureWrapMode::CLAMP_EDGE, TextureWrapMode::CLAMP_EDGE, TextureWrapMode::CLAMP_EDGE);