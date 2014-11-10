#include "stdafx.h"
#include "NightVisionRenderStage.h"

#include <ctime>

#include "colors.h"
#include "GLProgram.h"
#include "Options.h"
#include "Random.h"
#include "SamplerState.h"

NightVisionRenderStage::NightVisionRenderStage(vg::GLProgram* glProgram, vg::FullQuadVBO* quad) :
    _glProgram(glProgram),
    _quad(quad) {
    _texNoise.width = NIGHT_VISION_NOISE_QUALITY;
    _texNoise.height = NIGHT_VISION_NOISE_QUALITY;

    // Generate random data
    i32 pixCount = _texNoise.width * _texNoise.height;
    ui8* data = new ui8[pixCount];
    Random r(clock());
    for (i32 i = 0; i < pixCount; i++) {
        data[i] = (ui8)(r.genMT() * 255.0f);
    }

    // Build noise texture
    glGenTextures(1, &_texNoise.ID);
    glBindTexture(GL_TEXTURE_2D, _texNoise.ID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _texNoise.width, _texNoise.height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
    SamplerState::POINT_WRAP.set(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    delete[] data;
}
NightVisionRenderStage::~NightVisionRenderStage() {
    glDeleteTextures(1, &_texNoise.ID);
}

void NightVisionRenderStage::draw() {
    _et += NIGHT_VISION_DEFAULT_NOISE_TIME_STEP;

    //_visionColorHSL.r = fmod(_visionColorHSL.r = 0.005f, 6.28f);
    f32v3 _visionColor = color::convertHSLToRGB(_visionColorHSL);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _texNoise.ID);

    _glProgram->use();
    _glProgram->enableVertexAttribArrays();
    glUniform1i(_glProgram->getUniform("unTexColor"), NIGHT_VISION_TEXTURE_SLOT_COLOR);
    glUniform1i(_glProgram->getUniform("unTexNoise"), NIGHT_VISION_TEXTURE_SLOT_NOISE);
    glUniform1f(_glProgram->getUniform("unLuminanceExponent"), NIGHT_VISION_DEFAULT_LUMINANCE_THRESHOLD);
    glUniform1f(_glProgram->getUniform("unColorAmplification"), NIGHT_VISION_DEFAULT_COLOR_AMPLIFICATION);
    glUniform1f(_glProgram->getUniform("unTime"), _et);
    glUniform3f(_glProgram->getUniform("unVisionColor"), _visionColor.r, _visionColor.g, _visionColor.b);

    glDisable(GL_DEPTH_TEST);
    _quad->draw();
    glEnable(GL_DEPTH_TEST);

    _glProgram->disableVertexAttribArrays();
    _glProgram->unuse();
}
