#include "stdafx.h"
#include "NightVisionRenderStage.h"

#include <ctime>

#include <Vorb/colors.h>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/Random.h>
#include <Vorb/graphics/SamplerState.h>

#include "Options.h"

KEG_TYPE_DEF_SAME_NAME(NightVisionRenderParams, kt) {
    using namespace keg;
    kt.addValue("Color", Value::basic(offsetof(NightVisionRenderParams, color), BasicType::F32_V3));
    kt.addValue("Contrast", Value::basic(offsetof(NightVisionRenderParams, luminanceExponent), BasicType::F32));
    kt.addValue("Filter", Value::basic(offsetof(NightVisionRenderParams, luminanceTare), BasicType::F32));
    kt.addValue("Brightness", Value::basic(offsetof(NightVisionRenderParams, colorAmplification), BasicType::F32));
    kt.addValue("Noise", Value::basic(offsetof(NightVisionRenderParams, noisePower), BasicType::F32));
    kt.addValue("ColorNoise", Value::basic(offsetof(NightVisionRenderParams, noiseColor), BasicType::F32));
}

NightVisionRenderParams NightVisionRenderParams::createDefault() {
    NightVisionRenderParams v = {};
    v.colorAmplification = NIGHT_VISION_DEFAULT_COLOR_AMPLIFICATION;
    v.luminanceExponent = NIGHT_VISION_DEFAULT_LUMINANCE_EXPONENT;
    v.luminanceTare = NIGHT_VISION_DEFAULT_LUMINANCE_TARE;
    v.noisePower = NIGHT_VISION_DEFAULT_NOISE_POWER;
    v.noiseColor = NIGHT_VISION_DEFAULT_NOISE_COLOR;
    v.color = NIGHT_VISION_DEFAULT_VISION_COLOR;
    return v;
}

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
    glGenTextures(1, &_texNoise.id);
    glBindTexture(GL_TEXTURE_2D, _texNoise.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _texNoise.width, _texNoise.height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
    vg::SamplerState::POINT_WRAP.set(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    delete[] data;

    // Set Default Program Parameters
    _glProgram->use();
    glUniform1i(_glProgram->getUniform("unTexColor"), NIGHT_VISION_TEXTURE_SLOT_COLOR);
    glUniform1i(_glProgram->getUniform("unTexNoise"), NIGHT_VISION_TEXTURE_SLOT_NOISE);
    NightVisionRenderParams params = NightVisionRenderParams::createDefault();
    setParams(&params);
}
NightVisionRenderStage::~NightVisionRenderStage() {
    glDeleteTextures(1, &_texNoise.id);
}

void NightVisionRenderStage::draw() {
    _et += NIGHT_VISION_DEFAULT_NOISE_TIME_STEP;

    //_visionColorHSL.r = fmod(_visionColorHSL.r = 0.005f, 6.28f);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _texNoise.id);

    _glProgram->use();
    _glProgram->enableVertexAttribArrays();
    glUniform1f(_glProgram->getUniform("unTime"), _et);

    glDisable(GL_DEPTH_TEST);
    _quad->draw();
    glEnable(GL_DEPTH_TEST);

    _glProgram->disableVertexAttribArrays();
    vg::GLProgram::unuse();
}

void NightVisionRenderStage::setParams(NightVisionRenderParams* params) {
    _glProgram->use();
    glUniform1f(_glProgram->getUniform("unLuminanceExponent"), params->luminanceExponent);
    glUniform1f(_glProgram->getUniform("unLuminanceTare"), params->luminanceTare);
    glUniform1f(_glProgram->getUniform("unNoisePower"), params->noisePower);
    glUniform1f(_glProgram->getUniform("unNoiseColor"), params->noiseColor);
    glUniform1f(_glProgram->getUniform("unColorAmplification"), params->colorAmplification);
    glUniform3f(_glProgram->getUniform("unVisionColor"), params->color.r, params->color.g, params->color.b);
}

