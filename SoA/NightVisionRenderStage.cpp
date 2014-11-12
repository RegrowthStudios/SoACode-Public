#include "stdafx.h"
#include "NightVisionRenderStage.h"

#include <ctime>

#include "colors.h"
#include "GLProgram.h"
#include "Options.h"
#include "Random.h"
#include "SamplerState.h"

KEG_TYPE_INIT_BEGIN(NightVisionRenderParams, NightVisionRenderParams, kt)
using namespace Keg;
kt->addValue("Color", Value::basic(BasicType::F32_V3, offsetof(NightVisionRenderParams, colorHSL)));
kt->addValue("LuminanceExponent", Value::basic(BasicType::F32, offsetof(NightVisionRenderParams, luminanceExponent)));
kt->addValue("LuminanceTare", Value::basic(BasicType::F32, offsetof(NightVisionRenderParams, luminanceTare)));
kt->addValue("ColorAmplification", Value::basic(BasicType::F32, offsetof(NightVisionRenderParams, colorAmplification)));
KEG_ENUM_INIT_END

NightVisionRenderParams NightVisionRenderParams::createDefault() {
    NightVisionRenderParams v = {};
    v.colorAmplification = NIGHT_VISION_DEFAULT_COLOR_AMPLIFICATION;
    v.luminanceExponent = NIGHT_VISION_DEFAULT_LUMINANCE_THRESHOLD;
    v.colorHSL = NIGHT_VISION_DEFAULT_VISION_COLOR;
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
    glGenTextures(1, &_texNoise.ID);
    glBindTexture(GL_TEXTURE_2D, _texNoise.ID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _texNoise.width, _texNoise.height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
    SamplerState::POINT_WRAP.set(GL_TEXTURE_2D);
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
    glDeleteTextures(1, &_texNoise.ID);
}

void NightVisionRenderStage::draw() {
    _et += NIGHT_VISION_DEFAULT_NOISE_TIME_STEP;

    //_visionColorHSL.r = fmod(_visionColorHSL.r = 0.005f, 6.28f);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _texNoise.ID);

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
    glUniform1f(_glProgram->getUniform("unColorAmplification"), params->colorAmplification);
    f32v3 _visionColor = color::convertHSLToRGB(params->colorHSL);
    glUniform3f(_glProgram->getUniform("unVisionColor"), _visionColor.r, _visionColor.g, _visionColor.b);
}

